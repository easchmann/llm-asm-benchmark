#!/usr/bin/env bash
# Regression gate for the x86-only experiment (Phase 0 / 1).
#
# A thin wrapper around run_x86.sh that pins the model roster, repeat count, and cost
# cap so every tune/train change (prompt, sampling, repair budget) can be validated
# against the same measurement. Overrides are passed through as env vars, and a
# manifest of the gate configuration is written next to the aggregate so each run is
# self-describing.
#
# Usage:
#   MAX_REPAIRS=2 X86_REPEATS=3 ./scripts/run_gate.sh            # 3 repeats, default roster
#   X86_SKIP_REPAIR=1 ./scripts/run_gate.sh                       # single-shot baseline
#   X86_TEMPERATURE=0.3 X86_TOP_P=0.95 ./scripts/run_gate.sh     # decode sweep
#   GATE_KERNELS="atax bicg" GATE_REPEATS=1 X86_MODELS="Reasoning-Large:zai-org/GLM-5.3-Flash" ./scripts/run_gate.sh
#
# The gate runs a *single* grid per invocations (no internal repeat loop — callers
# choose their own budget and aggregate manually), and applies the shared ui.conf
# cost guardrail if ui.conf / ctx.sh are present, else falls back to printed $FLAG_CAP.
#
# Defaults:
#   kernels           = atax bicg mvt (override: GATE_KERNELS, space-separated)
#   repeats           = 1             (override: GATE_REPEATS; run_x86.sh's X86_REPEATS)
#   roster            = run_x86.sh's exact-model CATEGORY_MODELS (override: X86_MODELS)
#   repair / values   = run_x86.sh defaults except where overridden (X86_SKIP_REPAIR etc.)

set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

KERNELS=(${GATE_KERNELS:-atax bicg mvt})
REPEATS="${GATE_REPEATS:-1}"

[ -f "$ROOT/scripts/ui.conf" ] && . "$ROOT/scripts/ui.conf"
FLAG_CAP="${FLAG_CAP:-}"

echo "=== llm-asm gate @ $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
echo "kernels  : ${KERNELS[*]}"
echo "repeats  : $REPEATS"
echo "roster   : ${X86_MODELS:-<run_x86.sh default>}"
echo "skip-repair: ${X86_SKIP_REPAIR:-0}"
echo "temp/top_p/seed: ${X86_TEMPERATURE:-<default>} / ${X86_TOP_P:-<default>} / ${X86_SEED:-<default>}"
echo "repair/values: MAX_REPAIRS=${MAX_REPAIRS:-2} MAX_VALUES_REPAIRS=${MAX_VALUES_REPAIRS:-2}"
[ -n "$FLAG_CAP" ] && echo "cost cap : $FLAG_CAP"

export X86_REPEATS="$REPEATS"
export X86_RESULTS_BASE="${X86_RESULTS_BASE:-$ROOT/results}"
env X86_REPEATS="$REPEATS" X86_RESULTS_BASE="$X86_RESULTS_BASE" \
    MAX_REPAIRS="${MAX_REPAIRS:-2}" MAX_VALUES_REPAIRS="${MAX_VALUES_REPAIRS:-2}" \
    X86_SKIP_REPAIR="${X86_SKIP_REPAIR:-0}" \
    X86_TEMPERATURE="${X86_TEMPERATURE:-}" X86_TOP_P="${X86_TOP_P:-}" X86_SEED="${X86_SEED:-}" \
    "$ROOT/scripts/run_x86.sh" "${KERNELS[@]}"
