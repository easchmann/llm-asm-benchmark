#!/usr/bin/env python3
# progress of a running description->asm run, per model
# usage: python3 scripts/progress.py results/desc_full --repeats 5 [--kernels 24 --levels 4]

import argparse
import collections
import glob
import json
import os
import time

from gen_desc import load_records
from md import heading, table


def shards_per_model(run_dir):
    shards = collections.defaultdict(set)
    for path in glob.glob(os.path.join(run_dir, "calls", "*.jsonl")):
        for line in open(path):
            shards[json.loads(line)["model"]].add(path)
            break
    return shards


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dir")
    p.add_argument("--repeats", type=int, required=True)
    p.add_argument("--kernels", type=int, default=24)
    p.add_argument("--levels", type=int, default=4)
    args = p.parse_args()

    records = load_records(args.run_dir)
    expected = args.kernels * args.levels * args.repeats
    shards = shards_per_model(args.run_dir)
    by_model = collections.defaultdict(list)
    for r in records:
        by_model[r["model"]].append(r)

    rows = []
    for model, recs in sorted(by_model.items()):
        final = {r["cell"]: r for r in recs if r.get("stop_reason") and not (r["stop_reason"] == "api_error" and r["round"] == 0)}
        cost = sum(r.get("cost_usd") or 0 for r in recs)
        recent = recs[-30:]
        api_err = sum(r["stage"] == "no_answer" for r in recent) / len(recent)
        seconds = sum(r.get("elapsed_seconds") or 0 for r in recs)
        per_cell = seconds / len(final) if final else 0
        remaining = max(expected - len(final), 0)
        eta_h = remaining * per_cell / max(len(shards.get(model, ())), 1) / 3600
        last = max((r.get("time") or "" for r in recs), default="")
        rows.append([model, f"{len(final)}/{expected}", len(recs), f"${cost:.2f}", f"{api_err:.0%}",
                     f"{per_cell:.0f}", f"{eta_h:.1f} h", last[5:16].replace("T", " ")])
    heading(f"Progress of {args.run_dir} at {time.strftime('%m-%d %H:%M')}")
    table(["model", "done", "calls", "cost", "API errors (last 30)", "s per cell", "remaining", "last record"], rows)
    print(f"Total cost: ${sum(r.get('cost_usd') or 0 for r in records):.2f}")


if __name__ == "__main__":
    main()
