#!/usr/bin/env python3
# cross-tab repair outcomes: how many runs needed / were fixed by repair turns.
# Reads comparison.csv files with the trailing `repairs` column (written by the run
# scripts). usage: python3 scripts/report_repairs.py [results_dir ...]  (default: results/)

import csv
import os
import sys
from collections import defaultdict

DIRS = sys.argv[1:] or ["results/"]

def rows_for(d):
    path = os.path.join(d, "comparison.csv")
    if not os.path.exists(path):
        return None
    return list(csv.DictReader(open(path)))

by_dir = {}
for d in DIRS:
    if (rows := rows_for(d)) is None:
        print(f"{d}: no comparison.csv (skipping)")
    else:
        by_dir[d] = rows

# outcome by repairs made
tab = defaultdict(lambda: defaultdict(int))   # (dir, result) -> {repairs bucket}
buckets = ["0", "1", "2+"]
bucket_of = lambda r: "2+" if int(r or 0) >= 2 else (r or "0")

print("how many turns to reach each result:")
print(f"{'result':<12}" + "".join(f"{os.path.basename(d):>28}" for d in DIRS))
for result in ["pass", "mismatch", "build_fail", "no_output", "run_fail", "timeout", "api_error"]:
    cells = []
    for d in DIRS:
        rows = by_dir.get(d) or []
        subs = [bucket_of(r.get("repairs")) for r in rows if r["result"] == result]
        if not subs:
            cells.append("—")
        else:
            cells.append(" ".join(f"{b}:{subs.count(b)}" for b in buckets if b in subs))
    print(f"{result:<12}" + "".join(f"{c:>28}" for c in cells))

# rows that needed at least one repair
print("\nrows requiring repair (repairs>0):")
any_repair = False
for d in DIRS:
    rows = by_dir.get(d) or []
    for r in rows:
        if int(r.get("repairs") or 0) > 0:
            any_repair = True
            print(f"  {os.path.basename(d):<20} {r['alias']:<27} result={r['result']:<11} repairs={r['repairs']}")
if not any_repair:
    print("  (none yet)")
