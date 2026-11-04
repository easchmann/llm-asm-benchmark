#!/usr/bin/env python3
# Claude variants run through the claude CLI: thinking verified per call, tokens, time, notional cost and how much of
# the subscription's 5-hour and 7-day windows they used, extrapolated to a full grid of --cells cells per variant
# usage: python3 scripts/desc_claude.py results/desc_claude_pilot [--cells 480]
# window use is the sum of the increases of the utilization the CLI reports after each call, in time order; it counts
# all use of the subscription in that time (other Claude sessions too) and is only meaningful for variants run one by one

import argparse
import collections

from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table


def window_use(records, window):
    values = [(r["time"], (r.get("rate_limit") or {}).get(window)) for r in records]
    values = [v for _, v in sorted(values) if v is not None]
    return sum(max(b - a, 0) for a, b in zip(values, values[1:]))


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dirs", nargs="+")
    p.add_argument("--cells", type=int, default=480, help="cells per variant in the full grid")
    args = p.parse_args()

    variants = collections.defaultdict(list)
    for run_dir in args.run_dirs:
        for rounds in trajectories(load_records(run_dir)).values():
            variants[rounds[0]["model"]].append(rounds)

    heading("Thinking, per call")
    rows = []
    for model, cells in sorted(variants.items()):
        calls = [r for rs in cells for r in rs if r["stage"] != "no_answer"]
        thinking = [((r.get("usage") or {}).get("completion_tokens_details") or {}).get("reasoning_tokens") or 0 for r in calls]
        rows.append([model, ", ".join(sorted({str(r.get("underlying_model")) for r in calls})),
                     ", ".join(sorted({f"{(r.get('sampling_params') or {}).get('effort')}" for r in calls})),
                     len(calls), sum(t > 0 for t in thinking), sum(bool(r.get("thinking_blocks")) for r in calls),
                     f"{sum(thinking) / max(len(calls), 1):.0f}", max(thinking, default=0),
                     ", ".join(f"{k} {n}" for k, n in collections.Counter(r.get("finish_reason") for r in calls).items())])
    table(["variant", "answered by", "effort", "calls", "calls with thinking tokens", "calls with thinking blocks",
           "thinking tokens per call", "max", "finish reasons"], rows)

    heading(f"Cost per cell, and extrapolated to {args.cells} cells per variant")
    rows = []
    for model, cells in sorted(variants.items()):
        records = [r for rs in cells for r in rs]
        n = len(cells)
        seconds = [r.get("elapsed_seconds") or 0 for r in records]
        five, seven = window_use(records, "five_hour"), window_use(records, "seven_day")
        per_cell = sum(seconds) / n
        rows.append([model, n, sum(rs[0]["stage"] == "pass" for rs in cells), sum(rs[-1]["stage"] == "pass" for rs in cells),
                     sum(rs[0]["stage"] == "no_answer" for rs in cells), f"{len(records) / n:.2f}",
                     f"{sum((r.get('usage') or {}).get('completion_tokens') or 0 for r in records) / n:.0f}",
                     f"{sum(seconds) / max(len(records), 1):.0f}", f"{max(seconds, default=0):.0f}", f"{per_cell:.0f}",
                     f"${sum(r.get('cost_usd') or 0 for r in records) / n:.3f}",
                     f"{100 * five / n:.2f}%", f"{100 * seven / n:.2f}%",
                     f"{per_cell * args.cells / 3600:.1f} h", f"{five * args.cells / n:.1f}", f"{100 * seven * args.cells / n:.0f}%",
                     sum(r.get("limit_wait_seconds") or 0 for r in records) // 60])
    table(["variant", "cells", "pass first try", "pass after repair", "no answer", "calls per cell", "output tokens per cell",
           "s per call", "slowest call s", "s per cell", "notional cost per cell", "5-hour window per cell",
           "7-day window per cell", f"time for {args.cells} cells", f"5-hour windows for {args.cells}",
           f"7-day window for {args.cells}", "minutes waited on limits"], rows)


if __name__ == "__main__":
    main()
