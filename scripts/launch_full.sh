#!/usr/bin/env bash
# start the full description->asm run: one tmux session per shard (model + kernel group)
# usage: scripts/launch_full.sh <run-id> <repeats> [shard ...]    (no shard names = all shards)
# running it again resumes: finished cells are skipped, sessions that still exist are left alone
# DRY_RUN=1 prints the commands instead of starting sessions
# MAX_REPAIRS (default 5) and RUN_ARGS (extra run_desc options, e.g. "--temperature 0.6 --seed 42") apply to all shards

set -euo pipefail
[ $# -ge 2 ] || { echo "usage: $0 <run-id> <repeats> [shard ...]"; exit 1; }
RUN_ID=$1
REPEATS=$2
shift 2
ROOT=$(cd "$(dirname "$0")/.." && pwd)

G1="axpy atax gemm jacobi1d prefix_sum spmv dot gesummv"
G2="scal bicg 2mm jacobi2d histogram lower_bound nrm2 trmv"
G3="mvt gemver 3mm heat3d popcount sort8 syrk trmm"

declare -A MODEL=(
    [kimi-1]=moonshotai/Kimi-K3 [kimi-2]=moonshotai/Kimi-K3
    [glm-1]=zai-org/GLM-5.3-Flash [glm-2]=zai-org/GLM-5.3-Flash [glm-3]=zai-org/GLM-5.3-Flash
    [deepseek]=deepseek-ai/DeepSeek-V4-Flash
    [qwen]=Qwen/Qwen3.8-27B
)
declare -A KERNELS=(
    [kimi-1]="$G1 mvt gemver 3mm heat3d" [kimi-2]="$G2 popcount sort8 syrk trmm"
    [glm-1]="$G1" [glm-2]="$G2" [glm-3]="$G3"
    [deepseek]="$G1 $G2 $G3"
    [qwen]="$G1 $G2 $G3"
)
declare -A EXTRA=([qwen]="--reasoning-effort none")
ALL="kimi-1 kimi-2 glm-1 glm-2 glm-3 deepseek qwen"

[ "${DRY_RUN:-0}" = 1 ] || mkdir -p "$ROOT/results/$RUN_ID/logs"
for shard in ${@:-$ALL}; do
    [ -n "${MODEL[$shard]:-}" ] || { echo "unknown shard: $shard (known: $ALL)"; exit 1; }
    session="$RUN_ID-$shard"
    cmd="python3 scripts/run_desc.py --run-id $RUN_ID --repeats $REPEATS --levels L0 L1 L2 L3 \
--kernels ${KERNELS[$shard]} --models ${MODEL[$shard]} ${EXTRA[$shard]:-} \
--max-repairs ${MAX_REPAIRS:-5} --feedback compile+tests ${RUN_ARGS:-} --shard $shard 2>&1 | tee -a results/$RUN_ID/logs/$shard.out"
    if [ "${DRY_RUN:-0}" = 1 ]; then
        echo "[$session] $cmd"
        continue
    fi
    if tmux has-session -t "$session" 2>/dev/null; then
        busy=$(tmux list-panes -t "$session" -F "#{pane_current_command}" | grep -v -x -E "bash|sh|zsh" || true)
        if [ -n "$busy" ]; then
            echo "$session is still running ($busy), left alone"
            continue
        fi
        echo "$session exists but is idle, reusing it"
    else
        tmux new-session -d -s "$session" -c "$ROOT"
    fi
    tmux send-keys -t "$session" 'export RIVAULT_API_KEY=$(tr -d "\n" < ~/.rivault_key)' Enter
    tmux send-keys -t "$session" "$cmd" Enter
    echo "started $session"
done

echo
echo "watch:  python3 scripts/progress.py results/$RUN_ID --repeats $REPEATS"
echo "attach: tmux attach -t $RUN_ID-<shard>    stop one: tmux kill-session -t $RUN_ID-<shard>"
