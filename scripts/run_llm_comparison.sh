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

# Exact models by five categories (queried directly, no RiVault alias indirection).
# Mirrors run_x86.sh's roster — deduped to distinct underlying backends (probed
# 2026-09-29): DeepSeek-V4-Flash and DeepSeek-V4-Flash-reasoning resolve to the SAME
# vLLM backend, so -reasoning is replaced with moonshotai/Kimi-K3 (litellm_proxy/kimi-k3).
# Each element: "category:exact_model_id"
CATEGORY_MODELS=(
"Reasoning-Small:Qwen/Qwen3.8-27B"
"Reasoning-Medium:deepseek-ai/DeepSeek-V4-Flash"
"Reasoning-Medium:moonshotai/Kimi-K3"
"Reasoning-Large:zai-org/GLM-5.3-Flash"
"Reasoning-Large:zai-org/GLM-5.2"
"Agentic-Small:Qwen/Qwen3.6-35B-A3B"
)
# Override: `I8086_MODELS="cat:model cat:model ..."`
[ -n "$I8086_MODELS" ] && read -ra CATEGORY_MODELS <<< "$I8086_MODELS"

# sort into parallel arrays: MODEL[] id, CAT[] category
unset ALIASES CATS
declare -a ALIASES CATS
for pair in "${CATEGORY_MODELS[@]}"; do
    cat="${pair%%:*}"; model="${pair#*:}"
    ALIASES+=("$model"); CATS+=("$cat")
done

dry=""
kernels=()
for a in "$@"; do [ "$a" = "--dry-run" ] && dry="--dry-run" || kernels+=("$a"); done
[ ${#kernels[@]} -eq 0 ] && kernels=(atax bicg mvt)

# Repeated measures: REPEATS>1 re-runs the whole grid N times into sibling
# <run_id>_r<NN> dirs under results/llm_compare_<ts>_repeats/ (each repeat full and
# independent: own route snapshot, native build, status/JSONL), then aggregates them
# via scripts/aggregate_repeats.py. Same rationale as run_x86.sh: within-cell variance
# dominates the two 2026-09-29 x86 runs (10/15 kept-model cells flipped), so majority
# vote over N is needed for a stable measurement.
REPEATS="${I8086_REPEATS:-1}"
if [ "$REPEATS" -gt 1 ] && [ "${I8086_INNER:-0}" != "1" ]; then
    TS=$(date +%Y%m%d_%H%M%S)
    PARENT="$RESULTS_BASE/llm_compare_${TS}_repeats"
    mkdir -p "$PARENT"
    for r in $(seq -w 1 "$REPEATS"); do
        echo "=== repeat $r / $REPEATS -> $(basename "$PARENT")/r$r ==="
        I8086_INNER=1 RESULTS_BASE="$PARENT" RUN_ID="llm_compare_${TS}_r${r}" "$0" "$@"
    done
    echo
    echo "=== repeated-measures summary ==="
    if python3 "$ROOT/scripts/aggregate_repeats.py" "$PARENT" --out "$PARENT/repeated.csv"; then
        echo "aggregate: $PARENT/repeated.csv"
    else
        echo "(aggregation failed)"
    fi
    exit $?
fi

[ -f "$SUMMARY" ] || echo "kernel,alias,category,compiled,result,repairs" > "$SUMMARY"
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
    [ "${I8086_INNER:-0}" = "1" ] && echo "repeat: $(basename "$RESULTS")"
} > "$RESULTS/RUN.txt"

# Snapshot the gateway-resolved routing once at start, so a change mid-run can be
# detected and every result cited with its true model (same as run_x86.sh).
I8086_ROUTE_REV="${I8086_ROUTE_REV:-$(date +%Y%m%d_%H%M%S)}"
echo "route_rev: $I8086_ROUTE_REV" >> "$RESULTS/RUN.txt"
if [ "${I8086_SNAPSHOT_ROUTES:-1}" != "0" ]; then
    echo "alias\tresolved_model (snapshot @ $I8086_ROUTE_REV)" >> "$RESULTS/RUN.txt"
    python3 "$ROOT/scripts/llm_client.py" --resolve "${ALIASES[@]}" 2>/dev/null \
        >> "$RESULTS/RUN.txt" || echo "(route snapshot unavailable)" >> "$RESULTS/RUN.txt"
fi

for k in "${kernels[@]}"; do
    kdir=$(find "$SUITE" -type d -name "$k" | head -1)
    [ -z "$kdir" ] && { echo "$k: not found, skipping"; continue; }
    src="$kdir/$k.c"

    native="$RESULTS/${k}_native.txt"
    if [ ! -f "$native" ]; then
        gcc -O0 $flags "$UTIL/polybench.c" "$src" -o "$WORK/${k}_native" -lm
        "$WORK/${k}_native" > "$native"
    fi

    for i in "${!ALIASES[@]}"; do
        alias="${ALIASES[$i]}"
        category="${CATS[$i]}"
        slug=$(echo "$alias" | tr '/' '_')

        if grep -q "^$k,$category,$alias," "$SUMMARY" 2>/dev/null; then
            echo "=== $k / $category / $alias (already done, skipping) ==="
            continue
        fi
        echo "=== $k / $category / $alias ==="

        gen="$WORK/${k}__${slug}.c"
        python3 "$ROOT/scripts/generate_with_repair.py" --kernel "$k" --src "$src" --out "$gen" \
            --alias "$alias" --log "$RESULTS/llm_calls.jsonl" --prompt-template "$PROMPT" \
            --work-dir "$WORK" --cc ia16-elf-gcc --polybench-c "$UTIL/polybench.c" --kernel-dir "$kdir" \
            --extra-c-flags "-O0 -march=i8086 -mcmodel=small $flags" \
            --exe "$WORK/${k}__${slug}.exe" --max-repairs "${MAX_REPAIRS:-2}" $dry

        st="$WORK/${k}__${slug}.status"
        if [ ! -f "$st" ]; then
            echo "$k,$alias,$category,no,api_error,0" >> "$SUMMARY"
            continue
        fi
        read -r model compiled repairs <<< "$(python3 -c "import json,sys; d=json.load(open('$st')); print(d['underlying_model'] + ' yes ' + str(d['repairs']) if d['compiled'] else d['underlying_model'] + ' no ' + str(d['repairs']))")"

        # ship the generated code + snapshots + compile log with the results
        cp -f "$gen" "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".attempt*.c "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".compile_err.log "$CODE/" 2>/dev/null
        cp -f "$st" "$CODE/" 2>/dev/null

        if [ "$compiled" != "yes" ]; then
            echo "$k,$alias,$category,no,build_fail,$repairs" >> "$SUMMARY"
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
            diff -q "$native" "$out" >/dev/null && echo "$k,$alias,$category,yes,pass,$repairs" >> "$SUMMARY" \
                || echo "$k,$alias,$category,yes,mismatch,$repairs" >> "$SUMMARY"
        else
            echo "$k,$alias,$category,yes,no_output,$repairs" >> "$SUMMARY"
        fi
    done
done

echo
column -s, -t "$SUMMARY"
echo
echo "run saved in: $RESULTS"
echo "funnel view:     python3 scripts/funnel.py \"8086=$RESULTS:$WORK:8086\""
echo "repairs:         python3 scripts/report_repairs.py $RESULTS"
