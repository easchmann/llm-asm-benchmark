#!/usr/bin/env python3
# how consistent the k runs of each model-task pair are (task = kernel x level), first try only
# usage: python3 scripts/desc_consistency.py results/desc_det [--max-disagree 1] [--fast 1.0] [--list]
# suspected cache hits: answered records with cost 0 and an elapsed time below --fast seconds

import argparse
import collections

from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table


def pct(k, n):
    return f"{100 * k / n:.0f}%" if n else "-"


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dir")
    p.add_argument("--max-disagree", type=int, default=1, help="runs that may differ from the majority for a task to count as stable")
    p.add_argument("--fast", type=float, default=1.0, help="seconds below which a free answer counts as a suspected cache hit")
    p.add_argument("--list", action="store_true", help="also list every task that is not stable and every suspected cache hit")
    args = p.parse_args()

    records = load_records(args.run_dir)
    suspect = [r for r in records if r["stage"] != "no_answer" and not r.get("start_from")
               and r.get("cost_usd") == 0 and (r.get("elapsed_seconds") or 0) < args.fast]
    tasks = collections.defaultdict(list)
    for rounds in trajectories(records).values():
        r = rounds[0]
        if r["stage"] != "no_answer":
            tasks[r["model"], r["kernel"], r["level"]].append(r)

    rows, unstable = [], []
    for model in sorted({m for m, _, _ in tasks}):
        mine = {key: runs for key, runs in tasks.items() if key[0] == model}
        n = len(mine)
        same_code = same_stage = same_result = stable = majority_pass = all_pass = 0
        passes = runs_total = 0
        for key, runs in mine.items():
            results = [r["stage"] == "pass" for r in runs]
            agree = collections.Counter(results).most_common(1)[0][1]
            same_code += len({r.get("asm_sha") for r in runs}) == 1
            same_stage += len({r["stage"] for r in runs}) == 1
            same_result += len(set(results)) == 1
            stable += len(results) - agree <= args.max_disagree
            majority_pass += sum(results) * 2 > len(results)
            all_pass += all(results)
            passes += sum(results)
            runs_total += len(results)
            if len(results) - agree > args.max_disagree:
                unstable.append((key, f"{sum(results)}/{len(results)} pass", ", ".join(sorted({r["stage"] for r in runs}))))
        no_nonce = sum(not r.get("nonce", True) for runs in mine.values() for r in runs)
        rows.append([model, n, pct(no_nonce, runs_total), pct(same_code, n), pct(same_stage, n), pct(same_result, n),
                     pct(stable, n), pct(passes, runs_total), pct(majority_pass, n), pct(all_pass, n),
                     sum(r["model"] == model for r in suspect)])

    header = ["model", "tasks", "no nonce", "identical code", "same stage", "same pass/fail", f"stable (<= {args.max_disagree} differ)",
              "pass rate (all runs)", "majority pass", "all runs pass", "suspected cache hits"]
    heading("Consistency of the runs per model-task pair (first try)")
    table(header, rows)

    if args.list:
        heading("Tasks that are not stable")
        table(["model", "kernel", "level", "passed", "stages seen"],
              [[model, kernel, level, passed, stages] for (model, kernel, level), passed, stages in sorted(unstable)])
        heading(f"Suspected cache hits (cost 0, under {args.fast:g} s)")
        table(["cell", "round", "elapsed s", "cache headers"],
              [[r["cell"], r["round"], r.get("elapsed_seconds"), r.get("cache_headers") or "-"] for r in suspect])


if __name__ == "__main__":
    main()
