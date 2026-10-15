#!/usr/bin/env bash
# Launcher for the 8086 exact-model repeated-measures run
# Runs the 6-backend x 3-kernel experiment
# I8086_REPEATS times (default 5) in the background. Pass kernels to override.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
# shellcheck disable=SC1091
source scripts/env.sh
export RIVAULT_API_KEY="$(tr -d '[:space:]' < "$HOME/.rivault_key")"
export I8086_REPEATS="${I8086_REPEATS:-5}"
KERNELS=("${@:-atax bicg mvt}")
RUN_LOG="results/i8086_exact_launch_$(date +%Y%m%d_%H%M%S).log"
echo "log: $RUN_LOG"
nohup ./scripts/run_llm_comparison.sh "${KERNELS[@]}" > "$RUN_LOG" 2>&1 &
echo "pid: $!"
echo "watch: tail -f $RUN_LOG"
echo "aggregate: ls results/llm_compare_*_repeats/repeated.csv"
