#!/usr/bin/env python3
# compare two description->asm runs on the cells both answered (same model, kernel, level, repeat):
# pass rates with B - A and a 95% CI (bootstrap over kernels), first-try failure mix, cost and time, plans
# usage: python3 scripts/desc_compare.py results/desc_full results/desc_plan

import collections
import os
import random
import sys

from desc_plots import FAILURES, classify
from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table

LEVELS = ["L0", "L1", "L2", "L3"]


def short(model):
    return model.split("/")[-1]


def answered(run_dir):
    return {(c[0]["model"], c[0]["kernel"], c[0]["level"], c[0]["repeat"]): c
            for c in trajectories(load_records(run_dir)).values() if c[0]["stage"] != "no_answer"}


def rate(pairs, side, final):
    return sum((p[side][-1] if final else p[side][0])["stage"] == "pass" for p in pairs) / len(pairs)


def diff(pairs, final, n=2000, seed=0):
    by_kernel = collections.defaultdict(list)
    for p in pairs:
        by_kernel[p[0][0]["kernel"]].append(p)
    kernels = list(by_kernel)
    stat = lambda ps: rate(ps, 1, final) - rate(ps, 0, final)
    rng = random.Random(seed)
    values = sorted(stat([p for k in rng.choices(kernels, k=len(kernels)) for p in by_kernel[k]]) for _ in range(n))
    return f"{100 * stat(pairs):+.0f} pts [{100 * values[int(0.025 * n)]:+.0f}, {100 * values[int(0.975 * n)]:+.0f}]"


def pct(k, n):
    return f"{100 * k / n:.0f}%" if n else "-"


def main():
    dirs = sys.argv[1:3]
    names = [os.path.basename(os.path.normpath(d)) for d in dirs]
    a, b = map(answered, dirs)
    keys = sorted(a.keys() & b.keys())
    pairs = [(a[k], b[k]) for k in keys]
    models = sorted({k[0] for k in keys})
    print(f"A = {names[0]}, B = {names[1]}: {len(keys)} cells answered in both, "
          f"{len(a.keys() - b.keys())} only in A, {len(b.keys() - a.keys())} only in B\n")

    groups = [("all", "all", pairs)]
    for m in models:
        mine = [p for p in pairs if p[0][0]["model"] == m]
        groups += [(short(m), "all", mine)] + [(short(m), l, [p for p in mine if p[0][0]["level"] == l]) for l in LEVELS]
    groups += [("all", l, [p for p in pairs if p[0][0]["level"] == l]) for l in LEVELS]
    heading("Pass rate, B - A (95% CI, bootstrap over kernels)")
    table(["model", "level", "cells", "first try A", "first try B", "B - A", "after repair A", "after repair B", "B - A"],
          [[m, l, len(g)] + [x for final in (False, True)
                             for x in (f"{100 * rate(g, 0, final):.0f}%", f"{100 * rate(g, 1, final):.0f}%", diff(g, final))]
           for m, l, g in groups if g])

    heading("First try: pass and failure types (share of cells)")
    rows = []
    for m in models + [None]:
        for side, name in enumerate(names):
            firsts = [p[side][0] for p in pairs if m is None or p[0][0]["model"] == m]
            mix = collections.Counter("pass" if r["stage"] == "pass" else classify(r) for r in firsts)
            rows.append([short(m) if m else "all", name, len(firsts), pct(mix["pass"], len(firsts))]
                        + [pct(mix[code], len(firsts)) for _, code in FAILURES])
    table(["model", "run", "cells", "pass"] + [label for label, _ in FAILURES], rows)

    heading("Cost and time per cell (all rounds, the model's own plan call included)")
    rows = []
    for m in models:
        for side, name in enumerate(names):
            cells = [p[side] for p in pairs if p[0][0]["model"] == m]
            records = [r for c in cells for r in c]
            reused = [c[0] for c in cells if c[0].get("plan_reused")]
            plan_cost = sum(r.get("plan_cost_usd") or 0 for r in records)
            cost = sum(r.get("cost_usd") or 0 for r in records) + sum(r.get("plan_cost_usd") or 0 for r in reused)
            plan_seconds = sum(r.get("plan_elapsed_seconds") or 0 for r in records)
            seconds = sum(r.get("elapsed_seconds") or 0 for r in records) + sum(r.get("plan_elapsed_seconds") or 0 for r in reused)
            plans = sum("plan_usage" in c[0] for c in cells)
            calls = len(records) + plans
            solved = sum(c[-1]["stage"] == "pass" for c in cells)
            rows.append([short(m), name, len(cells), f"{calls / len(cells):.2f}", f"${cost / len(cells):.4f}",
                         f"${plan_cost / len(cells):.4f}" if plans else "-", f"{seconds / len(cells):.0f}",
                         f"{plan_seconds / len(cells):.0f}" if plans else "-",
                         f"${cost / solved:.4f}" if solved else "-"])
    table(["model", "run", "cells", "calls per cell", "cost per cell", "of which plan", "s per cell", "of which plan",
           "cost per solved cell"], rows)

    planned = [(name, p[side]) for p in pairs for side, name in enumerate(names) if "plan_sha" in p[side][0]]
    if planned:
        heading("Plans")
        rows = []
        for m in models:
            for name in names:
                cells = [c for n, c in planned if n == name and c[0]["model"] == m]
                if not cells:
                    continue
                code = [c for c in cells if c[0].get("plan_has_code")]
                clean = [c for c in cells if not c[0].get("plan_has_code")]
                first = lambda cs: pct(sum(c[0]["stage"] == "pass" for c in cs), len(cs))
                rows.append([short(m), name, len(cells), pct(len(code), len(cells)),
                             pct(sum(c[0].get("plan_finish_reason") == "length" for c in cells), len(cells)),
                             f"{sum(c[0].get('plan_chars') or 0 for c in cells) / len(cells):.0f}",
                             first(clean), first(code)])
        table(["model", "run", "plans", "with code", "cut off (length)", "mean chars", "first try: plan without code",
               "first try: plan with code"], rows)


if __name__ == "__main__":
    main()
