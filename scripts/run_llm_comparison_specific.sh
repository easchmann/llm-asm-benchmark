#!/usr/bin/env bash
# same as run_llm_comparison.sh but uses the more specific prompt andwrites to results_specific/ 
# usage: ./run_llm_comparison_specific.sh atax bicg mvt

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SUITE="$ROOT/kernels/polybench-c-4.2"
UTIL="$SUITE/utilities"
RESULTS="$ROOT/results_specific_2"
WORK="$ROOT/.work/llm_compare_specific_2"
FREEDOS_IMG="$ROOT/freedos/hdd.img"
PART_OFFSET=32256
SUMMARY="$RESULTS/comparison.csv"
PROMPT="$ROOT/prompts/i8086_prompt_specific.txt"

# chat/instruction models only
ALIASES=(RiVault/Reasoning-Tiny RiVault/Reasoning-Small RiVault/Reasoning-Medium RiVault/Reasoning-Large
         RiVault/Instruction-Tiny RiVault/Instruction-Small RiVault/Instruction-Medium
         RiVault/Agentic-Small RiVault/Agentic-Medium)

dry=""
kernels=()
for a in "$@"; do [ "$a" = "--dry-run" ] && dry="--dry-run" || kernels+=("$a"); done
[ ${#kernels[@]} -eq 0 ] && kernels=(atax bicg mvt)

mkdir -p "$RESULTS" "$WORK"
[ -f "$SUMMARY" ] || echo "kernel,alias,underlying_model,compiled,result" > "$SUMMARY"
flags="-DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS -I $UTIL"

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
        rm -f "$gen"
        python3 "$ROOT/scripts/generate_asm.py" --kernel "$k" --src "$src" --out "$gen" \
            --alias "$alias" --log "$RESULTS/llm_calls.jsonl" --prompt-template "$PROMPT" $dry

        model=$(tail -1 "$RESULTS/llm_calls.jsonl" | python3 -c "import json,sys; print(json.load(sys.stdin)['underlying_model'])" | tr ',' ';')

        if [ ! -f "$gen" ]; then
            echo "$k,$alias,$model,no,api_error" >> "$SUMMARY"
            continue
        fi

        if ! ia16-elf-gcc -O0 -march=i8086 -mcmodel=small $flags -I "$kdir" "$UTIL/polybench.c" "$gen" -o "$WORK/${k}__${slug}.exe" 2>"$WORK/${k}__${slug}.compile_err.log"; then
            echo "$k,$alias,$model,no,build_fail" >> "$SUMMARY"
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
            diff -q "$native" "$out" >/dev/null && echo "$k,$alias,$model,yes,pass" >> "$SUMMARY" \
                || echo "$k,$alias,$model,yes,mismatch" >> "$SUMMARY"
        else
            echo "$k,$alias,$model,yes,no_output" >> "$SUMMARY"
        fi
    done
done

echo
column -s, -t "$SUMMARY"