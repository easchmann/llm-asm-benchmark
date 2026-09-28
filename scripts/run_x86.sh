#!/usr/bin/env bash
# x86-64 control experiment: same task as the 8086 runs, but the generated
# inline asm is compiled with the node's own gcc and run natively (no emulator)
# usage: ./run_x86.sh atax bicg mvt

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SUITE="$ROOT/kernels/polybench-c-4.2"
UTIL="$SUITE/utilities"
RESULTS="${X86_RESULTS:-$ROOT/results_x86}"
WORK="${X86_WORK:-$ROOT/.work/x86}"
PROMPT="${X86_PROMPT:-$ROOT/prompts/x86_64_prompt_template.txt}"
SUMMARY="$RESULTS/comparison.csv"

# same models as the 8086 runs
ALIASES=(RiVault/Reasoning-Tiny RiVault/Reasoning-Small RiVault/Reasoning-Medium RiVault/Reasoning-Large
         RiVault/Instruction-Tiny RiVault/Instruction-Small RiVault/Instruction-Medium
         RiVault/Agentic-Small RiVault/Agentic-Medium)
[ -n "$X86_ALIASES" ] && read -ra ALIASES <<< "$X86_ALIASES"

# run on x86 node only
if [ "$(uname -m)" != "x86_64" ]; then
    echo "this machine is $(uname -m), need an x86_64 node (e.g. supercomp01a)"
    exit 1
fi
grep -q "POLYBENCH_DUMP_TARGET stdout" "$UTIL/polybench.h" || {
    echo "polybench.h isn't patched yet (dump goes to stderr)"
}
echo "running on $(hostname) ($(uname -m))"

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
        gcc -O0 $flags "$UTIL/polybench.c" "$src" -o "$WORK/${k}_native" -lm || { echo "native build failed"; continue; }
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

        exe="$WORK/${k}__${slug}"
        out="$RESULTS/${k}__${slug}_x86.txt"
        if ! gcc -O0 $flags -I "$kdir" "$UTIL/polybench.c" "$gen" -o "$exe" -lm 2>"$exe.compile_err.log"; then
            echo "$k,$alias,$model,no,build_fail" >> "$SUMMARY"
            continue
        fi

        # runs model-written code natively, cap time, cpu, memory and output size
        ( ulimit -t 30 -v 4000000 -f 1000; timeout 15s "$exe" < /dev/null > "$out" 2> "$exe.run_err.log" )
        rc=$?
        echo "exit code $rc" >> "$exe.run_err.log"

        if [ $rc -eq 124 ]; then result=timeout
        elif [ $rc -ne 0 ]; then result=run_fail
        elif diff -q "$native" "$out" >/dev/null; then result=pass
        else result=mismatch
        fi
        echo "$k,$alias,$model,yes,$result" >> "$SUMMARY"
    done
done

echo
column -s, -t "$SUMMARY"
echo
echo "finer grading:  python3 scripts/classify_outputs.py $RESULTS x86"