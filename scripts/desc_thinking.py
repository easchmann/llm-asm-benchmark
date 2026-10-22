#!/usr/bin/env python3
# reasoning on vs off per model, paired by kernel, level and repeat across runs and model labels:
# pass rates with on - off and a 95% CI (bootstrap over kernels), failure mix, tokens, time and cost per cell
# usage: python3 scripts/desc_thinking.py results/desc_full results/desc_thinking [--pair OFF ON ...]
# a variant is a model label in a run; without --pair the variants of each base model are paired automatically:
# the one whose calls report no reasoning tokens is "off", every other one is compared against it
# --pair takes labels such as zai-org/GLM-5.3-Flash@reasoning-none, or run:label where a label is in several runs

import argparse
import collections
import os
import random
import sys

from desc_plots import FAILURES, classify
from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table

LEVELS = ["L0", "L1", "L2", "L3"]


def reasoning_tokens(r):
    return ((r.get("usage") or {}).get("completion_tokens_details") or {}).get("reasoning_tokens") or 0


def load(run_dirs):
    variants = collections.defaultdict(dict)
    for run_dir in run_dirs:
        run = os.path.basename(os.path.normpath(run_dir))
        for rounds in trajectories(load_records(run_dir)).values():
            r = rounds[0]
            variants[run, r["model"]][r["kernel"], r["level"], r["repeat"]] = rounds
    return variants


def base_model(cells):
    r = next(iter(cells.values()))[0]
    return r.get("base_model") or r["api_model"]


def calls(cells):
    return [r for rs in cells.values() for r in rs if r["stage"] != "no_answer"]


def thinks(cells):
    answered = calls(cells)
    return sum(reasoning_tokens(r) > 0 for r in answered) > 0.05 * len(answered)


def name(variant):
    return f"{variant[1]} ({variant[0]})"


def find(label, variants):
    hits = [v for v in variants if label in (v[1], f"{v[0]}:{v[1]}")]
    if len(hits) != 1:
        sys.exit(f"--pair {label}: {len(hits)} matching variants, known: " + ", ".join(f"{r}:{m}" for r, m in sorted(variants)))
    return hits[0]


def auto_pairs(variants):
    by_base = collections.defaultdict(list)
    for v in sorted(variants):
        by_base[base_model(variants[v])].append(v)
    pairs = []
    for base, vs in sorted(by_base.items()):
        off = [v for v in vs if not thinks(variants[v])]
        on = [v for v in vs if thinks(variants[v])]
        if len(off) == 1:
            pairs += [(off[0], v) for v in on]
        elif on and off:
            print(f"{base}: {len(off)} variants without reasoning, choose one with --pair\n")
    return pairs


def passed(rounds, final):
    return (rounds[-1] if final else rounds[0])["stage"] == "pass"


def rate(pairs, side, final):
    return sum(passed(p[side], final) for p in pairs) / len(pairs)


def diff(pairs, final, n=2000, seed=0):
    by_kernel = collections.defaultdict(list)
    for p in pairs:
        by_kernel[p[0][0]["kernel"]].append(p)
    kernels = sorted(by_kernel)
    stat = lambda ps: rate(ps, 1, final) - rate(ps, 0, final)
    rng = random.Random(seed)
    values = sorted(stat([p for k in rng.choices(kernels, k=len(kernels)) for p in by_kernel[k]]) for _ in range(n))
    return f"{100 * stat(pairs):+.0f} pts [{100 * values[int(0.025 * n)]:+.0f}, {100 * values[int(0.975 * n)]:+.0f}]"


def pct(k, n):
    return f"{100 * k / n:.0f}%" if n else "-"


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dirs", nargs="+")
    p.add_argument("--pair", nargs=2, action="append", metavar=("OFF", "ON"), help="variant without and with reasoning")
    args = p.parse_args()

    variants = load(args.run_dirs)
    heading("Variants")
    table(["run", "model", "base model", "cells", "no answer", "reasoning tokens per call", "reasoning"],
          [[run, model, base_model(cells), len(cells), sum(rs[0]["stage"] == "no_answer" for rs in cells.values()),
            f"{sum(map(reasoning_tokens, calls(cells))) / max(len(calls(cells)), 1):.0f}",
            "on" if thinks(cells) else "off"] for (run, model), cells in sorted(variants.items())])

    chosen = [(find(off, variants), find(on, variants)) for off, on in args.pair] if args.pair else auto_pairs(variants)
    matched = {}
    for off, on in chosen:
        a, b = variants[off], variants[on]
        keys = sorted(k for k in a.keys() & b.keys()
                      if a[k][0]["stage"] != "no_answer" and b[k][0]["stage"] != "no_answer")
        matched[off, on] = [(a[k], b[k]) for k in keys]
    if not matched:
        sys.exit("no pairs of variants found; give them with --pair OFF ON")

    heading("Pass rate, on - off (95% CI, bootstrap over kernels)")
    rows = []
    for (off, on), pairs in matched.items():
        for level in ["all"] + LEVELS:
            g = [q for q in pairs if level == "all" or q[0][0]["level"] == level]
            if g:
                rows.append([name(off), name(on), level, len(g)] +
                            [x for final in (False, True)
                             for x in (f"{100 * rate(g, 0, final):.0f}%", f"{100 * rate(g, 1, final):.0f}%", diff(g, final))])
    table(["off", "on", "level", "cells", "first try off", "first try on", "on - off",
           "after repair off", "after repair on", "on - off"], rows)

    for when, index in (("First try", 0), ("After repair", -1)):
        heading(f"{when}: pass and failure types (share of cells)")
        rows = []
        for (off, on), pairs in matched.items():
            for side, v in enumerate((off, on)):
                mix = collections.Counter("pass" if q[side][index]["stage"] == "pass" else classify(q[side][index])
                                          for q in pairs)
                rows.append([name(v), "on" if side else "off", len(pairs), pct(mix["pass"], len(pairs))]
                            + [pct(mix[code], len(pairs)) for _, code in FAILURES])
        table(["variant", "reasoning", "cells", "pass"] + [label for label, _ in FAILURES], rows)

    heading("Tokens, time and cost per cell (all rounds)")
    rows = []
    for (off, on), pairs in matched.items():
        for side, v in enumerate((off, on)):
            cells = [q[side] for q in pairs]
            records = [r for rs in cells for r in rs]
            n = len(cells)
            usage = lambda key: sum((r.get("usage") or {}).get(key) or 0 for r in records) / n
            cost = sum(r.get("cost_usd") or 0 for r in records)
            solved = sum(rs[-1]["stage"] == "pass" for rs in cells)
            seconds = [r.get("elapsed_seconds") or 0 for r in records]
            rows.append([name(v), "on" if side else "off", n, f"{len(records) / n:.2f}",
                         f"{sum(map(reasoning_tokens, records)) / n:.0f}", f"{usage('completion_tokens'):.0f}",
                         f"{sum(seconds) / n:.0f}", f"{max(seconds):.0f}", sum(s >= 600 for s in seconds),
                         f"${cost / n:.4f}", f"${cost / solved:.4f}" if solved else "-"])
    table(["variant", "reasoning", "cells", "calls per cell", "reasoning tokens", "completion tokens", "s per cell",
           "slowest call s", "calls >= 600 s", "cost per cell", "cost per solved cell"], rows)


if __name__ == "__main__":
    main()
