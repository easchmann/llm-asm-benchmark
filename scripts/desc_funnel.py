#!/usr/bin/env python3
# final stage per cell, pass rate per model and level, cost per pass
# usage: python3 scripts/desc_funnel.py results/desc_pilot

import collections
import json
import math
import os
import sys

STAGES = ["no_answer", "extract_fail", "assemble_fail", "link_fail", "crash", "timeout", "wrong", "pass"]


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
    records = [json.loads(line) for line in open(os.path.join(sys.argv[1], "calls.jsonl"))]
    final = {r["cell"]: r for r in records if r.get("stop_reason")}
    first_try = {r["cell"] for r in records if r["round"] == 0 and r["stage"] == "pass"}

    stages = collections.defaultdict(collections.Counter)
    for r in final.values():
        stages[(r["model"], r["level"])][r["stage"]] += 1

    print(f"{'model':<32}{'level':<6}" + "".join(f"{s[:8]:>9}" for s in STAGES) + "   pass rate")
    for (model, level), c in sorted(stages.items()):
        n = sum(c.values())
        print(f"{model:<32}{level:<6}" + "".join(f"{c[s]:>9}" for s in STAGES) + f"   {rate(c['pass'], n)}")

    print("\nper level")
    for level in sorted({r["level"] for r in final.values()}):
        cells = [r for r in final.values() if r["level"] == level]
        passed = sum(r["stage"] == "pass" for r in cells)
        r0 = sum(r["cell"] in first_try for r in cells)
        print(f"{level:<6} final {rate(passed, len(cells)):<22} first try {rate(r0, len(cells))}")

    print("\nper model")
    cost = collections.Counter()
    for r in records:
        cost[r["model"]] += r.get("cost_usd") or 0
    for model in sorted(cost):
        passed = sum(r["stage"] == "pass" for r in final.values() if r["model"] == model)
        per_pass = f"${cost[model] / passed:.4f}" if passed else "-"
        print(f"{model:<32} passes {passed:<4} cost ${cost[model]:.4f}  per pass {per_pass}")


if __name__ == "__main__":
    main()
