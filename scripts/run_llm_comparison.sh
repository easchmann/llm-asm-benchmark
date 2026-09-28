#!/usr/bin/env bash
# generate + compile + run a kernel across several models, compare results
# usage: ./run_llm_comparison.sh atax bicg mvt

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SUITE="$ROOT/kernels/polybench-c-4.2"
UTIL="$SUITE/utilities"
FREEDOS_IMG="$ROOT/freedos/hdd.img"
PART_OFFSET=32256
PROMPT="${PROMPT:-$ROOT/prompts/i8086_prompt_template.txt}"

# each invocation gets a fresh, never-reused run directory under results/
# (override RESULTS_BASE, or RUN_ID to pin the exact name)
RESULTS_BASE="${RESULTS_BASE:-$ROOT/results}"
BASE="llm_compare"
RUN_ID="${RUN_ID:-${BASE}_$(date +%Y%m%d_%H%M%S)}"
n=0
while [ -d "$RESULTS_BASE/$RUN_ID" ]; do n=$((n+1)); RUN_ID="${RUN_ID}-$n"; done
RESULTS="$RESULTS_BASE/$RUN_ID"
WORK="$ROOT/.work/$RUN_ID"
CODE="$RESULTS/code"
SUMMARY="$RESULTS/comparison.csv"
mkdir -p "$RESULTS" "$WORK" "$CODE"

# chat/instruction models only
ALIASES=(RiVault/Reasoning-Tiny RiVault/Reasoning-Small RiVault/Reasoning-Medium RiVault/Reasoning-Large
         RiVault/Instruction-Tiny RiVault/Instruction-Small RiVault/Instruction-Medium
         RiVault/Agentic-Small RiVault/Agentic-Medium)

dry=""
kernels=()
for a in "$@"; do [ "$a" = "--dry-run" ] && dry="--dry-run" || kernels+=("$a"); done
[ ${#kernels[@]} -eq 0 ] && kernels=(atax bicg mvt)

[ -f "$SUMMARY" ] || echo "kernel,alias,underlying_model,compiled,result,repairs" > "$SUMMARY"
flags="-DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -I $UTIL"

# describe this run inside its own directory
{
    echo "run_id: $RUN_ID"
    echo "started: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "results: $RESULTS"
    echo "work: $WORK"
    echo "prompt: $PROMPT"
    echo "kernel(s): ${kernels[*]}"
    echo "models: ${ALIASES[*]}"
    echo "command: $0 $*"
} > "$RESULTS/RUN.txt"

for k in "${kernels[@]}"; do
    kdir=$(find "$SUITE" -type d -name "$k" | head -1)
    [ -z "$kdir" ] && { echo "$k: not found, skipping"; continue; }
    src="$kdir/$k.c"

    native="$RESULTS/${k}_native.txt"
    if [ ! -f "$native" ]; then
        gcc -O0 $flags "$UTIL/polybench.c" "$src" -o "$WORK/${k}_native" -lm
        "$WORK/${k}_native" > "$native"
    fi

    for alias in "${ALIASES[@]}"; do
        slug=$(echo "$alias" | tr '/' '_')

        if grep -q "^$k,$alias," "$SUMMARY" 2>/dev/null; then
            echo "=== $k / $alias (already done, skipping) ==="
            continue
        fi
        echo "=== $k / $alias ==="

        gen="$WORK/${k}__${slug}.c"
        python3 "$ROOT/scripts/generate_with_repair.py" --kernel "$k" --src "$src" --out "$gen" \
            --alias "$alias" --log "$RESULTS/llm_calls.jsonl" --prompt-template "$PROMPT" \
            --work-dir "$WORK" --cc ia16-elf-gcc --polybench-c "$UTIL/polybench.c" --kernel-dir "$kdir" \
            --extra-c-flags "-O0 -march=i8086 -mcmodel=small $flags" \
            --exe "$WORK/${k}__${slug}.exe" --max-repairs "${MAX_REPAIRS:-2}" $dry

        st="$WORK/${k}__${slug}.status"
        if [ ! -f "$st" ]; then
            model=$(tail -1 "$RESULTS/llm_calls.jsonl" | python3 -c "import json,sys; print(json.load(sys.stdin).get('underlying_model','?'))" | tr ',' ';')
            echo "$k,$alias,$model,no,api_error,0" >> "$SUMMARY"
            continue
        fi
        read -r model compiled repairs <<< "$(python3 -c "import json,sys; d=json.load(open('$st')); print(d['underlying_model'] + ' yes ' + str(d['repairs']) if d['compiled'] else d['underlying_model'] + ' no ' + str(d['repairs']))")"

        # ship the generated code + snapshots + compile log with the results
        cp -f "$gen" "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".attempt*.c "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".compile_err.log "$CODE/" 2>/dev/null
        cp -f "$st" "$CODE/" 2>/dev/null

        if [ "$compiled" != "yes" ]; then
            echo "$k,$alias,$model,no,build_fail,$repairs" >> "$SUMMARY"
            continue
        fi

        img="$WORK/${k}__${slug}.img"
        cp "$FREEDOS_IMG" "$img"
        name=$(echo "$k" | tr a-z A-Z | cut -c1-8).EXE
        mcopy -o -i "$img@@$PART_OFFSET" "$WORK/${k}__${slug}.exe" "::$name"
        printf "%s > OUTPUT.TXT\nPOWEROFF\n" "$name" > "$WORK/${k}__${slug}.bat"
        mcopy -o -i "$img@@$PART_OFFSET" "$WORK/${k}__${slug}.bat" "::FDAUTO.BAT"
        mcopy -o -i "$img@@$PART_OFFSET" "$WORK/${k}__${slug}.bat" "::AUTOEXEC.BAT"

        qemu-system-i386 -drive file="$img",format=raw,if=ide -boot c -nographic -no-reboot \
            -monitor telnet:127.0.0.1:4444,server,nowait < /dev/null > "$WORK/${k}__${slug}.log" 2>&1 &
        qemu_pid=$!
        sleep 10
        exec 3<>/dev/tcp/127.0.0.1/4444
        echo "quit" >&3
        exec 3<&-
        wait "$qemu_pid" 2>/dev/null

        out="$RESULTS/${k}__${slug}_8086.txt"
        if mcopy -i "$img@@$PART_OFFSET" "::OUTPUT.TXT" "$out" 2>/dev/null; then
            diff -q "$native" "$out" >/dev/null && echo "$k,$alias,$model,yes,pass,$repairs" >> "$SUMMARY" \
                || echo "$k,$alias,$model,yes,mismatch,$repairs" >> "$SUMMARY"
        else
            echo "$k,$alias,$model,yes,no_output,$repairs" >> "$SUMMARY"
        fi
    done
done

echo
column -s, -t "$SUMMARY"
echo
echo "run saved in: $RESULTS"
echo "funnel view:     python3 scripts/funnel.py \"8086=$RESULTS:$WORK:8086\""
echo "repairs:         python3 scripts/report_repairs.py $RESULTS"
