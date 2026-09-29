#!/usr/bin/env python3
# cross-tab repair outcomes: how many compiles-error turns and (new) values-feedback
# turns each row needed, and whether values-repair moved a row from mismatch -> pass.
#
# Reads comparison.csv (has the trailing `repairs` column) plus, where available, the
# per-row status file <dir>/code/<kernel>__<alias>.status for values_repairs / result
# (both written by run_x86.sh / generate_with_repair.py).
#
# usage: python3 scripts/report_repairs.py [results_dir ...]  (default: results/)

import csv
import json
import os
import sys
from collections import defaultdict

DIRS = sys.argv[1:] or ["results/"]


def rows_for(d):
    path = os.path.join(d, "comparison.csv")
    if not os.path.exists(path):
        return None
    return list(csv.DictReader(open(path)))


def status_for(d, kernel, alias):
    """Read <dir>/code/<kernel>__<alias>.status (shipped alongside results)."""
    slug = alias.replace("/", "_")
    p = os.path.join(d, "code", f"{kernel}__{slug}.status")
    if not os.path.exists(p):
        return {}
    try:
        return json.load(open(p))
    except Exception:
        return {}


by_dir = {}
for d in DIRS:
    if (rows := rows_for(d)) is None:
        print(f"{d}: no comparison.csv (skipping)")
    else:
        by_dir[d] = rows

buckets = ["0", "1", "2+"]
bucket_of = lambda r: "2+" if int(r or 0) >= 2 else (r or "0")

# --- table 1: how many compiler-error repair turns to each result -----------------
print("how many compiler-repair turns to reach each result:")
print(f"{'result':<12}" + "".join(f"{os.path.basename(d):>30}" for d in DIRS))
for result in ["pass", "mismatch", "build_fail", "no_output", "run_fail", "timeout", "api_error"]:
    cells = []
    for d in DIRS:
        rows = by_dir.get(d) or []
        subs = [bucket_of(r.get("repairs")) for r in rows if r["result"] == result]
        cells.append(" ".join(f"{b}:{subs.count(b)}" for b in buckets if b in subs) or "—")
    print(f"{result:<12}" + "".join(f"{c:>30}" for c in cells))

# --- table 2: values-feedback turns per result (new) ------------------------------
print("\nhow many values-feedback turns to each result (status values_repairs):")
print(f"{'result':<12}" + "".join(f"{os.path.basename(d):>30}" for d in DIRS))
for result in ["pass", "mismatch", "build_fail", "no_output", "run_fail", "timeout", "api_error"]:
    cells = []
    for d in DIRS:
        rows = by_dir.get(d) or []
        subs = [bucket_of(status_for(d, r["kernel"], r["alias"]).get("values_repairs"))
                or "0" for r in rows if r["result"] == result]
        cells.append(" ".join(f"{b}:{subs.count(b)}" for b in buckets if b in subs) or "—")
    print(f"{result:<12}" + "".join(f"{c:>30}" for c in cells))

# --- table 3: rows where values-feedback actually ran (values_repairs > 0) --------
print("\nrows where a values-feedback turn ran (values_repairs>0):")
any_v = False
for d in DIRS:
    rows = by_dir.get(d) or []
    for r in rows:
        vr = status_for(d, r["kernel"], r["alias"]).get("values_repairs") or 0
        if int(vr) > 0:
            any_v = True
            print(f"  {os.path.basename(d):<24} {r['kernel']:<5} {r['alias']:<27} "
                  f"final={r['result']:<10} compiler_repairs={r.get('repairs')} "
                  f"values_repairs={vr}")
if not any_v:
    print("  (none yet — values-check not run or no row needed it)")

# --- table 4: rows that needed at least one compiler repair -----------------------
print("\nrows requiring compiler repair (repairs>0):")
any_repair = False
for d in DIRS:
    rows = by_dir.get(d) or []
    for r in rows:
        if int(r.get("repairs") or 0) > 0:
            any_repair = True
            print(f"  {os.path.basename(d):<24} {r['kernel']:<5} {r['alias']:<27} "
                  f"result={r['result']:<10} repairs={r['repairs']}")
if not any_repair:
    print("  (none yet)")
