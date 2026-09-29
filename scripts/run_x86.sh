#!/usr/bin/env bash
# x86-64 control experiment: same task as the 8086 runs, but the generated
# inline asm is compiled with the node's own gcc and run natively (no emulator)
# usage: ./run_x86.sh atax bicg mvt

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SUITE="$ROOT/kernels/polybench-c-4.2"
UTIL="$SUITE/utilities"
PROMPT="${X86_PROMPT:-$ROOT/prompts/x86_64_prompt_template.txt}"

# each invocation gets a fresh, never-reused run directory under results/
RESULTS_BASE="${X86_RESULTS_BASE:-$ROOT/results}"
BASE="x86"
RUN_ID="${RUN_ID:-${BASE}_$(date +%Y%m%d_%H%M%S)}"
n=0
while [ -d "$RESULTS_BASE/$RUN_ID" ]; do n=$((n+1)); RUN_ID="${RUN_ID}-$n"; done
RESULTS="$RESULTS_BASE/$RUN_ID"
WORK="$ROOT/.work/$RUN_ID"
CODE="$RESULTS/code"
SUMMARY="$RESULTS/comparison.csv"
mkdir -p "$RESULTS" "$WORK" "$CODE"

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
        # --values-check: after a compile-pass the Python loop itself runs+grades (against
        # native) and repairs wrong OUTPUT values (not just compiler errors). 8086 path
        # does NOT pass this (its run is qemu/FreeDOS inside bash) and keeps bash grading.
        # On by default for x86 ("1"); set X86_VALUES_CHECK=0 to disable.
        values_args=""
        if [ "${X86_VALUES_CHECK:-1}" != "0" ]; then
            values_args="--values-check --native $native"
        fi
        python3 "$ROOT/scripts/generate_with_repair.py" --kernel "$k" --src "$src" --out "$gen" \
            --alias "$alias" --log "$RESULTS/llm_calls.jsonl" --prompt-template "$PROMPT" \
            --work-dir "$WORK" --cc gcc --polybench-c "$UTIL/polybench.c" --kernel-dir "$kdir" \
            --extra-c-flags "$flags" --link-math \
            --exe "$WORK/${k}__${slug}" --max-repairs "${MAX_REPAIRS:-2}" \
            --max-values-repairs "${MAX_VALUES_REPAIRS:-2}" ${values_args:+$values_args} $dry

        st="$WORK/${k}__${slug}.status"
        if [ ! -f "$st" ]; then
            model=$(tail -1 "$RESULTS/llm_calls.jsonl" | python3 -c "import json,sys; print(json.load(sys.stdin).get('underlying_model','?'))" | tr ',' ';')
            echo "$k,$alias,$model,no,api_error,0" >> "$SUMMARY"
            continue
        fi
        vcheck="${X86_VALUES_CHECK:-1}"
        read -r model compiled repairs raw_result <<< "$(python3 -c "
import json,sys
d=json.load(open('$st'))
if d['compiled']:
    print(d['underlying_model'], 'yes', d['repairs'], d.get('result') or '')
else:
    print(d['underlying_model'], 'no', d['repairs'])")"
        # When values-check ran, the Python loop already graded -> trust its result
        # (pass/mismatch/empty/no_output/timeout/run_fail). Otherwise fall back to bash grading.
        if [ "$vcheck" != "0" ] && [ "$compiled" = "yes" ]; then
            result="$raw_result"
            [ -z "$result" ] || [ "$result" = "None" ] && result="run_fail"
        else
            result=""
        fi

        # ship the generated code + snapshots + compile log + (values snapshots) with the results
        cp -f "$gen" "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".attempt*.c "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".values*.c "$CODE/" 2>/dev/null
        cp -f "$WORK/${k}__${slug}".compile_err.log "$CODE/" 2>/dev/null
        cp -f "$st" "$CODE/" 2>/dev/null

        if [ "$compiled" != "yes" ]; then
            echo "$k,$alias,$model,no,build_fail,$repairs" >> "$SUMMARY"
            continue
        fi

        exe="$WORK/${k}__${slug}"
        out="$RESULTS/${k}__${slug}_x86.txt"

        # fallback (values-check disabled, or no status result): run + grade here as before
        if [ -z "$result" ]; then
            # prefer Python's pre-captured values_check.txt (from the values loop) if present
            rc=""
            if [ -s "$WORK/${k}__${slug}.values_check.txt" ]; then
                cp -f "$WORK/${k}__${slug}.values_check.txt" "$out"
                rc=0   # it ran successfully inside Python
            else
                ( ulimit -t 30 -v 4000000 -f 1000; timeout 15s "$exe" < /dev/null > "$out" 2> "$exe.run_err.log" )
                rc=$?
                echo "exit code $rc" >> "$exe.run_err.log"
            fi
            if [ "$rc" -eq 124 ]; then result=timeout
            elif [ "$rc" -ne 0 ]; then result=run_fail
            elif diff -q "$native" "$out" >/dev/null; then result=pass
            else result=mismatch
            fi
        fi
        echo "$k,$alias,$model,yes,$result,$repairs" >> "$SUMMARY"
    done
done

echo
column -s, -t "$SUMMARY"
echo
echo
echo "funnel view:     python3 scripts/funnel.py \"x86=$RESULTS:$WORK:x86\""
echo "repairs:         python3 scripts/report_repairs.py $RESULTS"
