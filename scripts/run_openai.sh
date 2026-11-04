#!/usr/bin/env bash
# run an OpenAI model on the same grid and settings as desc_full (24 kernels, L0-L3, 5 repeats, up to 5 repairs)
# usage: scripts/run_openai.sh <model> [run-id] [extra run_desc options, e.g. --reasoning-effort low]
# the key is read from ~/.openai_key inside the tmux session and never printed; DRY_RUN=1 prints the command

set -euo pipefail
[ $# -ge 1 ] || { echo "usage: $0 <model> [run-id] [extra run_desc options]"; exit 1; }
MODEL=$1
RUN_ID=${2:-desc_full}
shift $(( $# >= 2 ? 2 : 1 ))
ROOT=$(cd "$(dirname "$0")/.." && pwd)
KERNELS=$(cd "$ROOT/kernels/desc" && ls -d */ | tr -d / | grep -v -x common | tr '\n' ' ')
SHARD="openai-$(echo "$MODEL" | tr '/.:' '---')"
SESSION="$RUN_ID-$SHARD"

cmd="RIVAULT_BASE_URL=https://api.openai.com/v1 python3 scripts/run_desc.py --run-id $RUN_ID --repeats 5 \
--levels L0 L1 L2 L3 --kernels $KERNELS --models $MODEL --max-repairs 5 --feedback compile+tests \
--shard $SHARD $* 2>&1 | tee -a results/$RUN_ID/logs/$SHARD.out"

if [ "${DRY_RUN:-0}" = 1 ]; then
    echo "[$SESSION] $cmd"
    exit 0
fi
[ -s ~/.openai_key ] || { echo "put the OpenAI key in ~/.openai_key first (chmod 600)"; exit 1; }
mkdir -p "$ROOT/results/$RUN_ID/logs"
if tmux has-session -t "$SESSION" 2>/dev/null; then
    echo "$SESSION exists; attach with: tmux attach -t $SESSION"
    exit 0
fi
tmux new-session -d -s "$SESSION" -c "$ROOT"
tmux send-keys -t "$SESSION" 'export RIVAULT_API_KEY=$(tr -d "\n" < ~/.openai_key)' Enter
tmux send-keys -t "$SESSION" "$cmd" Enter
echo "started $SESSION"
echo "watch: python3 scripts/progress.py results/$RUN_ID --repeats 5"
