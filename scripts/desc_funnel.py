#!/usr/bin/env python3
# final stage per cell, pass rate per model and level, cost per pass
# usage: python3 scripts/desc_funnel.py results/desc_pilot

import collections
import math
import sys

from desc_repair import C_STAGES
from gen_desc import load_records
from md import heading, table

STAGES = ["no_answer", "extract_fail", "forbidden", "incomplete", "compile_fail", "assemble_fail", "link_fail", "crash",
          "timeout", "wrong", "pass"]


def wilson(k, n, z=1.96):
    if n == 0:
        return 0.0, 0.0
    p = k / n
    d = 1 + z * z / n
    c = (p + z * z / (2 * n)) / d
    h = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d
    return c - h, c + h


def rate(k, n):
    lo, hi = wilson(k, n)
    return f"{k}/{n} [{lo:.2f}-{hi:.2f}]"


def main():
    records = load_records(sys.argv[1])
    final = {r["cell"]: r for r in records if r.get("stop_reason")}
    first_try = {r["cell"] for r in records if r["round"] == 0 and r["stage"] == "pass"}

    stages = collections.defaultdict(collections.Counter)
    for r in final.values():
        stages[(r["model"], r["level"])][r["stage"]] += 1

    shown = [s for s in STAGES if s not in C_STAGES or any(r["stage"] == s for r in final.values())]
    heading("Final stage per model and level")
    table(["model", "level"] + shown + ["pass rate"],
          [[model, level] + [c[s] for s in shown] + [rate(c["pass"], sum(c.values()))]
           for (model, level), c in sorted(stages.items())])

    heading("Per level")
    rows = []
    for level in sorted({r["level"] for r in final.values()}):
        cells = [r for r in final.values() if r["level"] == level]
        passed = sum(r["stage"] == "pass" for r in cells)
        r0 = sum(r["cell"] in first_try for r in cells)
        rows.append([level, rate(r0, len(cells)), rate(passed, len(cells))])
    table(["level", "first try", "final"], rows)

    heading("Per model")
    cost = collections.Counter()
    for r in records:
        cost[r["model"]] += r.get("cost_usd") or 0
    rows = []
    for model in sorted(cost):
        passed = sum(r["stage"] == "pass" for r in final.values() if r["model"] == model)
        rows.append([model, passed, f"${cost[model]:.4f}", f"${cost[model] / passed:.4f}" if passed else "-"])
    table(["model", "passes", "cost", "cost per pass"], rows)


if __name__ == "__main__":
    main()
