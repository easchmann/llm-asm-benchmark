#!/usr/bin/env bash
# start one tmux session that runs Claude variants through the claude CLI (LLM_BACKEND=claude-cli), one after another:
# a variant is finished (all cells done, or no progress in 3 passes 30 min apart) before the next one starts
# usage: scripts/run_claude.sh <run-id> <repeats> <model:on|off>... [-- extra run_desc options]
#   e.g. scripts/run_claude.sh desc_claude 5 claude-opus-5-5:on claude-opus-5:on claude-opus-5:off
#        scripts/run_claude.sh desc_claude_pilot 1 claude-haiku-5-5:on -- --kernels axpy gemm --levels L0 L3
# without --kernels/--levels the grid of run_model.sh; MAX_REPAIRS (default 5), SESSION (default <run-id>-claude), DRY_RUN=1
# usage limits do not stop the run: the backend sleeps until the reset and retries (see scripts/claude_cli.py)
# CLAUDE_CLI_* variables set when starting (e.g. CLAUDE_CLI_PAUSE_AT=0.8 CLAUDE_CLI_RESUME_AFTER=...) are passed on
# running it again resumes: finished cells are skipped, a session that still runs is left alone

set -euo pipefail
[ $# -ge 3 ] || { echo "usage: $0 <run-id> <repeats> <model:on|off>... [-- extra run_desc options]"; exit 1; }
RUN_ID=$1
REPEATS=$2
shift 2
ROOT=$(cd "$(dirname "$0")/.." && pwd)
VARIANTS=()
while [ $# -gt 0 ] && [ "$1" != "--" ]; do VARIANTS+=("$1"); shift; done
[ $# -gt 0 ] && shift
EXTRA=("$@")
SESSION=${SESSION:-$RUN_ID-claude}
KERNELS="axpy atax gemm jacobi1d prefix_sum spmv dot gesummv scal bicg 2mm jacobi2d histogram lower_bound nrm2 trmv \
mvt gemver 3mm heat3d popcount sort8 syrk trmm"

if [ "${INSIDE:-0}" = 1 ]; then
    export LLM_BACKEND=claude-cli
    unset ANTHROPIC_API_KEY
    for variant in "${VARIANTS[@]}"; do
        model=${variant%:*}
        thinking=${variant##*:}
        shard=$(printf '%s' "${model#claude-}-$thinking" | tr -c 'a-z0-9-' '-')
        args=(--run-id "$RUN_ID" --repeats "$REPEATS" --levels L0 L1 L2 L3 --kernels $KERNELS --models "$model"
              --thinking "$thinking" --max-repairs "${MAX_REPAIRS:-5}" --feedback compile+tests "${EXTRA[@]}" --shard "$shard")
        stalls=0
        while true; do
            left=$(python3 scripts/run_desc.py "${args[@]}" --remaining)
            echo "[$(date '+%m-%d %H:%M')] $variant: $left cells to do"
            [ "$left" = 0 ] && break
            python3 scripts/run_desc.py "${args[@]}" || true
            after=$(python3 scripts/run_desc.py "${args[@]}" --remaining)
            [ "$after" = 0 ] && continue
            [ "$after" -lt "$left" ] && stalls=0 || stalls=$((stalls + 1))
            [ $stalls -ge 3 ] && { echo "[$(date '+%m-%d %H:%M')] $variant: no progress in 3 passes, moving on"; break; }
            echo "[$(date '+%m-%d %H:%M')] $variant: $after cells left after this pass, retrying in 30 min"
            sleep 1800
        done
    done
    echo "[$(date '+%m-%d %H:%M')] all variants done"
    exit 0
fi

cmd="$(env | grep '^CLAUDE_CLI_' | sed 's/=\(.*\)/="\1"/' | tr '\n' ' ')INSIDE=1 MAX_REPAIRS=${MAX_REPAIRS:-5} scripts/run_claude.sh $(printf '%q ' "$RUN_ID" "$REPEATS" "${VARIANTS[@]}" -- "${EXTRA[@]}")\
2>&1 | tee -a results/$RUN_ID/logs/$SESSION.out"
if [ "${DRY_RUN:-0}" = 1 ]; then
    echo "[$SESSION] $cmd"
    exit 0
fi
mkdir -p "$ROOT/results/$RUN_ID/logs"
if tmux has-session -t "$SESSION" 2>/dev/null; then
    busy=$(tmux list-panes -t "$SESSION" -F "#{pane_current_command}" | grep -v -x -E "bash|sh|zsh" || true)
    if [ -n "$busy" ]; then
        echo "$SESSION is still running ($busy), left alone"
        exit 0
    fi
    echo "$SESSION exists but is idle, reusing it"
else
    tmux new-session -d -s "$SESSION" -c "$ROOT"
fi
tmux send-keys -t "$SESSION" "$cmd" Enter
echo "started $SESSION: ${VARIANTS[*]}"
echo
echo "watch:  python3 scripts/progress.py results/$RUN_ID --repeats $REPEATS; grep usage-limit results/$RUN_ID/logs/$SESSION.out | tail"
echo "attach: tmux attach -t $SESSION    stop: tmux kill-session -t $SESSION"
