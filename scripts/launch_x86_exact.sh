#!/usr/bin/env bash
# Launcher for the x86 exact-model repeated-measures run
# Runs the 6-backend x 3-kernel experiment X86_REPEATS times (default 5) in the background
# with output to a per-run log. Pass kernels to override: ./launch_x86_exact.sh atax
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
export RIVAULT_API_KEY="$(tr -d '[:space:]' < "$HOME/.rivault_key")"
export X86_REPEATS="${X86_REPEATS:-5}"
KERNELS=("${@:-atax bicg mvt}")
RUN_LOG="results/x86_exact_launch_$(date +%Y%m%d_%H%M%S).log"
echo "launching x86 exact-model repeated run: kernels=${KERNELS[*]} repeats=$X86_REPEATS"
echo "log: $RUN_LOG"
nohup ./scripts/run_x86.sh "${KERNELS[@]}" > "$RUN_LOG" 2>&1 &
echo "pid: $!"
echo "watch: tail -f $RUN_LOG"
echo "aggregate: ls results/x86_*_repeats/repeated.csv"
