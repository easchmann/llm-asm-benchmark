#!/usr/bin/env python3
# what the repair rounds did: pass rate by round, trajectory classes, stage transitions, repair cost
# usage: python3 scripts/desc_repair.py results/desc_full [--by model|level|kernel|category]

import argparse
import collections

from gen_desc import load_records
from md import heading, table

ORDER = ["extract_fail", "incomplete", "assemble_fail", "link_fail", "crash", "timeout", "wrong", "pass"]
RANK = {s: i for i, s in enumerate(ORDER)}
CLASSES = ["first try", "fixed", "progressing", "oscillating", "stuck", "regressing", "interrupted",
           "not repaired", "no answer"]
CATEGORY = {
    "axpy": "blas1", "dot": "blas1", "scal": "blas1", "nrm2": "blas1",
    "atax": "blas2", "bicg": "blas2", "mvt": "blas2", "gemver": "blas2", "gesummv": "blas2", "trmv": "blas2",
    "gemm": "blas3", "2mm": "blas3", "3mm": "blas3", "syrk": "blas3", "trmm": "blas3",
    "jacobi1d": "stencil", "jacobi2d": "stencil", "heat3d": "stencil",
    "prefix_sum": "integer", "histogram": "integer", "popcount": "integer",
    "spmv": "irregular", "lower_bound": "irregular", "sort8": "irregular",
}


def trajectories(records):
    attempts = collections.defaultdict(list)
    for r in records:
        if r["round"] == 0:
            attempts[r["cell"]].append([])
        if attempts[r["cell"]]:
            attempts[r["cell"]][-1].append(r)
    cells = {}
    for cell, tries in attempts.items():
        complete = [t for t in tries if t[-1].get("stop_reason")]
        if complete:
            cells[cell] = complete[-1]
    return cells


def score(r):
    return RANK[r["stage"]] * 10 + (r.get("sizes_passed") or 0)


def classify(rounds):
    if rounds[0]["stage"] == "no_answer":
        return "no answer"
    if rounds[0]["stage"] == "pass":
        return "first try"
    if rounds[-1]["stage"] == "pass":
        return "fixed"
    answered = [r for r in rounds if r["stage"] != "no_answer"]
    if len(answered) < len(rounds):
        return "interrupted"
    if len(answered) == 1:
        return "not repaired"
    scores = [score(r) for r in answered]
    ups = any(b > a for a, b in zip(scores, scores[1:]))
    downs = any(b < a for a, b in zip(scores, scores[1:]))
    if ups and downs:
        return "oscillating"
    if scores[-1] > scores[0]:
        return "progressing"
    if scores[-1] < scores[0]:
        return "regressing"
    return "stuck"


def group_key(rounds, by):
    r = rounds[0]
    if by == "model":
        return r["model"]
    if by == "category":
        return (r["model"], CATEGORY.get(r["kernel"], "other"))
    return (r["model"], r[by])


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dir")
    p.add_argument("--by", choices=["model", "level", "kernel", "category"], default="model")
    args = p.parse_args()

    cells = trajectories(load_records(args.run_dir))
    groups = collections.defaultdict(list)
    for rounds in cells.values():
        groups[group_key(rounds, args.by)].append(rounds)
    max_round = max((len(r) - 1 for r in cells.values()), default=0)

    name = lambda key: " / ".join(key) if isinstance(key, tuple) else key
    ordered = sorted(groups.items(), key=lambda kv: str(kv[0]))

    heading("Cumulative pass rate by round (cells with an answer in round 0)")
    rows = []
    for key, cell_rounds in ordered:
        answered = [rs for rs in cell_rounds if rs[0]["stage"] != "no_answer"]
        if answered:
            rows.append([name(key), len(answered)] +
                        [f"{100 * sum(any(r['stage'] == 'pass' and r['round'] <= k for r in rs) for rs in answered) / len(answered):.0f}%"
                         for k in range(max_round + 1)])
    table([args.by, "cells"] + [f"r{k}" for k in range(max_round + 1)], rows)

    heading("Trajectory classes")
    table([args.by] + CLASSES, [[name(key)] + [collections.Counter(classify(rs) for rs in cell_rounds)[c] for c in CLASSES]
                                for key, cell_rounds in ordered])

    heading("Stage transitions between consecutive rounds (all cells)")
    moves = collections.Counter()
    for rounds in cells.values():
        for a, b in zip(rounds, rounds[1:]):
            moves[(a["stage"], b["stage"])] += 1
    table(["from", "to", "count"], [[a, b, n] for (a, b), n in moves.most_common(20)])

    heading("Repair cost per model")
    rows = []
    by_model = collections.defaultdict(list)
    for rounds in cells.values():
        by_model[rounds[0]["model"]].append(rounds)
    for model, cell_rounds in sorted(by_model.items()):
        fixed = [rs for rs in cell_rounds if classify(rs) == "fixed"]
        repair = [r for rs in cell_rounds for r in rs if r["round"] > 0]
        cost = sum(r.get("cost_usd") or 0 for r in repair)
        mean_round = sum(rs[-1]["round"] for rs in fixed) / len(fixed) if fixed else 0
        per_fix = f"${cost / len(fixed):.4f}" if fixed else "-"
        rows.append([model, len(fixed), f"{mean_round:.1f}", len(repair), f"${cost:.3f}", per_fix])
    table(["model", "fixed", "mean round", "repair calls", "repair cost", "cost per fix"], rows)

    heading("Stop reasons")
    stops = collections.Counter(rs[-1].get("stop_reason") for rs in cells.values())
    table(["stop reason", "cells"], [[s, n] for s, n in stops.most_common()])


if __name__ == "__main__":
    main()
