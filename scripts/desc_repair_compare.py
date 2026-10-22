#!/usr/bin/env python3
# paired comparison of two repair runs on the same cells, e.g. a --start-from run against its source run
# usage: python3 scripts/desc_repair_compare.py results/desc_fb_v2 results/desc_full
# only cells that failed round 0 in both runs are compared; round 0 must be the same answer (same asm_sha)

import argparse
import collections
import re

from desc_repair import CLASSES, classify, score, trajectories
from gen_desc import load_records
from md import heading, table


def pct(k, n):
    return f"{100 * k / n:.0f}%" if n else "-"


def first_error(r):
    return re.sub(r"/tmp/cc\w+\.s", "<tmp>.s", r["first_error"] or "")


def metrics(cell_rounds):
    pairs = [(a, b) for rs in cell_rounds for a, b in zip(rs, rs[1:]) if b["stage"] != "no_answer"]
    asm = [(a, b) for a, b in pairs if a["stage"] == b["stage"] == "assemble_fail"]
    crashes = [(a, b) for a, b in pairs if a["stage"] == "crash"]
    multi = [rs for rs in cell_rounds if len(rs) > 2]
    return {
        "same assembler error repeated": pct(sum(first_error(a) == first_error(b) for a, b in asm), len(asm)),
        "crash fixed in next round": pct(sum(b["stage"] != "crash" and score(b) > score(a) for a, b in crashes), len(crashes)),
        "earlier failure comes back": pct(sum(recurs(rs) for rs in multi), len(multi)),
    }


def recurs(rounds):
    sigs = [r.get("error_sig") for r in rounds]
    return any(s in sigs[:k - 1] and s != sigs[k - 1] for k, s in enumerate(sigs) if k >= 2)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("new_run")
    p.add_argument("base_run")
    args = p.parse_args()

    new = trajectories(load_records(args.new_run))
    base = trajectories(load_records(args.base_run))
    failed = lambda rs: rs[0]["stage"] not in ("pass", "no_answer")
    shared = sorted(c for c in new if c in base and failed(new[c]) and failed(base[c]))
    changed = [c for c in shared if new[c][0].get("asm_sha") != base[c][0].get("asm_sha")
               or new[c][0]["stage"] != base[c][0]["stage"]]
    print(f"{len(shared)} cells failed round 0 in both runs; {len(changed)} of them have a different round 0 "
          f"(answer or regraded stage), {len([c for c in new if c not in base])} cells are only in {args.new_run}\n")

    by_model = collections.defaultdict(list)
    for c in shared:
        by_model[new[c][0]["model"]].append(c)
    runs = [(args.base_run, base), (args.new_run, new)]

    heading("Pass after repair on the same failed first answers")
    rows = []
    for model, cells in sorted(by_model.items()):
        fixed = {name: {c for c in cells if traj[c][-1]["stage"] == "pass"} for name, traj in runs}
        b, n = fixed[args.base_run], fixed[args.new_run]
        rows.append([model, len(cells), pct(len(b), len(cells)), pct(len(n), len(cells)),
                     len(b & n), len(b - n), len(n - b)])
    table(["model", "cells", f"fixed ({args.base_run})", f"fixed ({args.new_run})",
           "fixed in both", "only base", "only new"], rows)

    heading("Trajectory classes")
    rows = []
    for model, cells in sorted(by_model.items()):
        for name, traj in runs:
            counts = collections.Counter(classify(traj[c]) for c in cells)
            rows.append([model, name] + [counts[k] for k in CLASSES])
    table(["model", "run"] + CLASSES, rows)

    heading("Repair behaviour")
    rows = []
    for model, cells in sorted(by_model.items()):
        for name, traj in runs:
            rounds = [traj[c] for c in cells]
            repair = [r for rs in rounds for r in rs if r["round"] > 0]
            stops = collections.Counter(rs[-1].get("stop_reason") for rs in rounds)
            rows.append([model, name, *metrics(rounds).values(), stops["same_errors"], len(repair),
                         f"${sum(r.get('cost_usd') or 0 for r in repair):.3f}"])
    table(["model", "run", "same assembler error repeated", "crash fixed in next round", "earlier failure comes back",
           "stopped: same errors", "repair calls", "repair cost"], rows)


if __name__ == "__main__":
    main()
