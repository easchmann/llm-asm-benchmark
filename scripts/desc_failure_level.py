#!/usr/bin/env python3
# does the type of failure depend on the description level? per model, first try and after repair
# chi-square with a permutation p-value (levels shuffled within each kernel), Cramer's V, standardized residuals
# usage: python3 scripts/desc_failure_level.py results/desc_full [--permutations 5000]

import argparse
import collections
import math
import random

from desc_plots import FAILURES, LEVELS, MODELS, classify
from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table

NAMES = dict((code, name) for name, code in FAILURES)


def chi2(pairs, cats):
    counts = collections.Counter(pairs)
    n = len(pairs)
    rows = collections.Counter(l for l, _ in pairs)
    cols = collections.Counter(c for _, c in pairs)
    stat, resid = 0.0, {}
    for l in LEVELS:
        for c in cats:
            e = rows[l] * cols[c] / n
            if e:
                stat += (counts[l, c] - e) ** 2 / e
                resid[l, c] = (counts[l, c] - e) / math.sqrt(e)
    return stat, resid, counts


def permutation_p(failures, cats, observed, n_perm, seed=0):
    by_kernel = collections.defaultdict(list)
    for kernel, level, cat in failures:
        by_kernel[kernel].append((level, cat))
    rng = random.Random(seed)
    hits = 0
    for _ in range(n_perm):
        pairs = []
        for items in by_kernel.values():
            levels = [l for l, _ in items]
            rng.shuffle(levels)
            pairs += [(l, c) for l, (_, c) in zip(levels, items)]
        hits += chi2(pairs, cats)[0] >= observed - 1e-9
    return (hits + 1) / (n_perm + 1)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dir")
    p.add_argument("--permutations", type=int, default=5000)
    args = p.parse_args()

    cells = [c for c in trajectories(load_records(args.run_dir)).values() if c[0]["stage"] != "no_answer"]
    summary, details = [], []
    for when, index in (("first try", 0), ("after repair", -1)):
        for model, label, _ in MODELS:
            failures = [(c[0]["kernel"], c[0]["level"], classify(c[index]))
                        for c in cells if c[0]["model"] == model and c[index]["stage"] != "pass"]
            cats = [code for _, code in FAILURES if any(f[2] == code for f in failures)]
            n = len(failures)
            if n < 20 or len(cats) < 2:
                summary.append([label, when, n, "-", "-", "-", "too few failures"])
                continue
            stat, resid, counts = chi2([(l, c) for _, l, c in failures], cats)
            v = math.sqrt(stat / (n * (min(len(LEVELS), len(cats)) - 1)))
            pval = permutation_p(failures, cats, stat, args.permutations)
            strongest = sorted(resid.items(), key=lambda kv: -abs(kv[1]))[:3]
            summary.append([label, when, n, f"{stat:.1f}", f"{v:.2f}", f"{pval:.4f}",
                            "; ".join(f"{NAMES[c]} at {l} {'more' if r > 0 else 'less'} ({r:+.1f})"
                                      for (l, c), r in strongest)])
            details.append((f"{label}, {when} (n={n})", cats, counts, resid))

    heading("Association between description level and failure type")
    table(["model", "when", "failures", "chi-square", "Cramer's V", "p (permutation within kernel)",
           "largest deviations (standardized residual)"], summary)

    for title, cats, counts, resid in details:
        heading(f"{title}: count (standardized residual)")
        table(["level"] + [NAMES[c] for c in cats],
              [[l] + [f"{counts[l, c]} ({resid.get((l, c), 0):+.1f})" for c in cats] for l in LEVELS])


if __name__ == "__main__":
    main()
