#!/usr/bin/env python3
# what the repair rounds did: pass rate by round, trajectory classes, stage transitions, repair cost
# usage: python3 scripts/desc_repair.py results/desc_full [--by model|level|kernel|category]

import argparse
import collections

from gen_desc import load_records

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

    print("cumulative pass rate by round (cells with an answer in round 0)")
    print(f"{'group':<44}{'cells':>6}" + "".join(f"{'r' + str(k):>7}" for k in range(max_round + 1)))
    for key, cell_rounds in sorted(groups.items(), key=lambda kv: str(kv[0])):
        answered = [rs for rs in cell_rounds if rs[0]["stage"] != "no_answer"]
        if not answered:
            continue
        row = []
        for k in range(max_round + 1):
            passed = sum(any(r["stage"] == "pass" and r["round"] <= k for r in rs) for rs in answered)
            row.append(f"{100 * passed / len(answered):6.0f}%")
        print(f"{str(key):<44}{len(answered):>6}" + " ".join(row))

    print("\ntrajectory classes")
    print(f"{'group':<44}" + "".join(f"{c[:11]:>12}" for c in CLASSES))
    for key, cell_rounds in sorted(groups.items(), key=lambda kv: str(kv[0])):
        counts = collections.Counter(classify(rs) for rs in cell_rounds)
        print(f"{str(key):<44}" + "".join(f"{counts[c]:>12}" for c in CLASSES))

    print("\nstage transitions between consecutive rounds (all cells)")
    moves = collections.Counter()
    for rounds in cells.values():
        for a, b in zip(rounds, rounds[1:]):
            moves[(a["stage"], b["stage"])] += 1
    for (a, b), n in moves.most_common(20):
        print(f"  {a:>14} -> {b:<14}{n:>6}")

    print("\nrepair cost per model")
    print(f"{'model':<44}{'fixed':>7}{'mean round':>12}{'repair calls':>14}{'repair cost':>13}{'per fix':>10}")
    by_model = collections.defaultdict(list)
    for rounds in cells.values():
        by_model[rounds[0]["model"]].append(rounds)
    for model, cell_rounds in sorted(by_model.items()):
        fixed = [rs for rs in cell_rounds if classify(rs) == "fixed"]
        repair = [r for rs in cell_rounds for r in rs if r["round"] > 0]
        cost = sum(r.get("cost_usd") or 0 for r in repair)
        mean_round = sum(rs[-1]["round"] for rs in fixed) / len(fixed) if fixed else 0
        per_fix = f"${cost / len(fixed):.4f}" if fixed else "-"
        print(f"{model:<44}{len(fixed):>7}{mean_round:>12.1f}{len(repair):>14}{'$%.3f' % cost:>13}{per_fix:>10}")

    stops = collections.Counter(rs[-1].get("stop_reason") for rs in cells.values())
    print("\nstop reasons:", dict(stops))


if __name__ == "__main__":
    main()
