#!/usr/bin/env bash
# start one tmux session that runs the description->asm grid of launch_full.sh for a single model or setting
# usage: scripts/run_model.sh <run-id> <repeats> <model> [extra run_desc options]
#   e.g. scripts/run_model.sh desc_thinking 5 zai-org/GLM-5.3-Flash --reasoning-effort none
#        scripts/run_model.sh desc_thinking 5 RiVault/Reasoning-Medium --base-model deepseek-ai/DeepSeek-V4-Flash --levels L0 L3
# extra --kernels/--levels replace the full grid; MAX_REPAIRS (default 5), RUN_ARGS and DRY_RUN=1 work as in launch_full.sh
# SHARD names the session and log (default: model name plus reasoning effort), e.g. to split one model over sessions
# running it again resumes: finished cells are skipped, a session that still runs is left alone

set -euo pipefail
[ $# -ge 3 ] || { echo "usage: $0 <run-id> <repeats> <model> [extra run_desc options]"; exit 1; }
RUN_ID=$1
REPEATS=$2
MODEL=$3
shift 3
ROOT=$(cd "$(dirname "$0")/.." && pwd)

KERNELS="axpy atax gemm jacobi1d prefix_sum spmv dot gesummv scal bicg 2mm jacobi2d histogram lower_bound nrm2 trmv \
mvt gemver 3mm heat3d popcount sort8 syrk trmm"
effort=$(echo " $* ${RUN_ARGS:-} " | sed -n 's/.* --reasoning-effort \([^ ]*\) .*/-\1/p')
shard=${SHARD:-$(printf '%s' "${MODEL##*/}$effort" | tr 'A-Z' 'a-z' | tr -c 'a-z0-9-' '-')}
session="$RUN_ID-$shard"
extra=$([ $# -gt 0 ] && printf '%q ' "$@" || true)
cmd="python3 scripts/run_desc.py --run-id $RUN_ID --repeats $REPEATS --levels L0 L1 L2 L3 --kernels $KERNELS \
--models $MODEL --max-repairs ${MAX_REPAIRS:-5} --feedback compile+tests $extra${RUN_ARGS:-} --shard $shard \
2>&1 | tee -a results/$RUN_ID/logs/$shard.out"

if [ "${DRY_RUN:-0}" = 1 ]; then
    echo "[$session] $cmd"
    exit 0
fi
mkdir -p "$ROOT/results/$RUN_ID/logs"
if tmux has-session -t "$session" 2>/dev/null; then
    busy=$(tmux list-panes -t "$session" -F "#{pane_current_command}" | grep -v -x -E "bash|sh|zsh" || true)
    if [ -n "$busy" ]; then
        echo "$session is still running ($busy), left alone"
        exit 0
    fi
    echo "$session exists but is idle, reusing it"
else
    tmux new-session -d -s "$session" -c "$ROOT"
fi
tmux send-keys -t "$session" 'export RIVAULT_API_KEY=$(tr -d "\n" < ~/.rivault_key)' Enter
tmux send-keys -t "$session" "$cmd" Enter
echo "started $session"
echo
echo "watch:  python3 scripts/progress.py results/$RUN_ID --repeats $REPEATS"
echo "attach: tmux attach -t $session    stop: tmux kill-session -t $session"
