#!/usr/bin/env python3
"""Majority-vote aggregation across a repeated-measures x86 run.

Takes the parent directory produced by run_x86.sh with X86_REPEATS>1 (a tree of
<parent>/<run_id>/ with per-run comparison.csv on the inner runs) and emits a
kernel x created summary CSV.

Each CSV row: kernel, alias, category, runs_ok, result, votes, pass_n, mismatch_n,
run_fail_n, build_fail_n, no_output_n, timeout_n, api_error_n, agreement

  result      = majority vote across the N repeats ("tie" if no unique winner)
  runs        = number of repeats with a recorded result for this cell
  agreement   = fraction of those picks equal to the majority (1.0 = unanimous)
  votes       = "pass 3/5" style summary

Usage:
  python3 scripts/aggregate_repeats.py PARENT_DIR [--out out.csv]
"""
import argparse
import csv
import glob
import os
import sys

RESULT_ORDER = ["pass", "mismatch", "no_output", "run_fail", "build_fail",
                "timeout", "api_error"]


def iter_cell_rows(parent):
    """Yield (result, rep_label, cell_row dict) for every cell in every repeat run."""
    for run in sorted(glob.glob(os.path.join(parent, "x86_*"))):
        csvp = os.path.join(run, "comparison.csv")
        if not os.path.isfile(csvp):
            continue
        for row in csv.DictReader(open(csvp)):
            # skip any aggregator / stray rows
            if not row.get("kernel"):
                continue
            yield row, os.path.basename(run)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("parent", help="parent dir containing x86_*/repeat run dirs")
    ap.add_argument("--out", default=None, help="output CSV path (default: repeated.csv next to parent)")
    args = ap.parse_args()

    cells = {}
    n_runs = 0
    for row, rep in iter_cell_rows(args.parent):
        n_runs = max(n_runs, 0)  # count dirs below separately
        key = (row["kernel"], row["alias"], row.get("category", ""))
        cells.setdefault(key, []).append(row["result"])
    run_dirs = sorted(glob.glob(os.path.join(args.parent, "x86_*")))
    n_runs = len(run_dirs)

    outp = args.out or os.path.join(args.parent, "repeated.csv")
    with open(outp, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["kernel", "alias", "category", "runs_ok", "result", "votes",
                    "pass_n", "mismatch_n", "no_output_n", "run_fail_n", "build_fail_n",
                    "timeout_n", "api_error_n", "agreement"])
        for (kernel, alias, cat), res in sorted(cells.items()):
            counts = {r: res.count(r) for r in RESULT_ORDER}
            n = len(res)
            best, best_n = None, 0
            for r in RESULT_ORDER:
                if counts[r] > best_n:
                    best, best_n = r, counts[r]
            winner = best if best_n * 2 > n else "tie"
            votes = " ".join(f"{r}={counts[r]}" for r in RESULT_ORDER if counts[r])
            agreement = (best_n / n) if n else 0.0
            w.writerow([kernel, alias, cat, n, winner, votes,
                        counts["pass"], counts["mismatch"], counts["no_output"],
                        counts["run_fail"], counts["build_fail"], counts["timeout"],
                        counts["api_error"], f"{agreement:.2f}"])
    print(f"aggregated {n_runs} repeat dirs -> {os.path.basename(outp)} ({len(cells)} cells)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
