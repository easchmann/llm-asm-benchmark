#!/usr/bin/env python3
# analysis of a description->asm run: pass rates with kernel-bootstrap CIs, level effect, failure reasons,
# kernel difficulty, PolyBench vs other, assembler errors, checks on passing code
# usage: python3 scripts/desc_analysis.py results/desc_full

import collections
import os
import random
import re
import sys

from desc_repair import trajectories
from gen_desc import load_records
from md import heading, table

POLYBENCH = {"atax", "bicg", "mvt", "gemm", "gemver", "2mm", "3mm", "gesummv", "syrk", "trmm",
             "jacobi1d", "jacobi2d", "heat3d"}
LEVELS = ["L0", "L1", "L2", "L3"]


def short(model):
    return model.split("/")[-1]


def passed(cell, final):
    return (cell[-1] if final else cell[0])["stage"] == "pass"


def rate(cells, final):
    return sum(passed(c, final) for c in cells) / len(cells) if cells else float("nan")


def bootstrap(cells, stat, n=2000, seed=0):
    by_kernel = collections.defaultdict(list)
    for c in cells:
        by_kernel[c[0]["kernel"]].append(c)
    kernels = list(by_kernel)
    rng = random.Random(seed)
    values = sorted(stat([c for k in rng.choices(kernels, k=len(kernels)) for c in by_kernel[k]]) for _ in range(n))
    return values[int(0.025 * n)], values[int(0.975 * n)]


def fmt(x, ci=None):
    return f"{100 * x:.0f}%" + (f" [{100 * ci[0]:.0f}-{100 * ci[1]:.0f}]" if ci else "")


def reason(r):
    if r["stage"] == "crash":
        return f"crash {r.get('signal')}"
    if r["stage"] != "wrong":
        return r["stage"]
    text = " ".join(r.get("test_failures") or [])
    if "callee-saved" in text or "stack pointer" in text:
        return "wrong: ABI"
    if "outside the array" in text:
        return "wrong: out-of-bounds write"
    if "input was modified" in text:
        return "wrong: input modified"
    sizes = list((r.get("sizes") or {}).values())
    if sizes and sizes[0] == "pass" and "fail" in sizes:
        return "wrong: small sizes pass"
    if sizes and all(s == "fail" for s in sizes):
        return "wrong: all sizes"
    return "wrong: other"


def main():
    run_dir = sys.argv[1]
    cells = [c for c in trajectories(load_records(run_dir)).values() if c[0]["stage"] != "no_answer"]
    models = sorted({c[0]["model"] for c in cells})
    of = lambda m, **kw: [c for c in cells if c[0]["model"] == m and all(c[0][k] == v for k, v in kw.items())]

    heading("Pass rate (95% CI, bootstrap over kernels)")
    table(["model", "cells", "first try", "after repair"],
          [[short(m), len(of(m)), fmt(rate(of(m), False), bootstrap(of(m), lambda x: rate(x, False))),
            fmt(rate(of(m), True), bootstrap(of(m), lambda x: rate(x, True)))] for m in models])

    heading("Level effect: L3 minus L0, paired by kernel (95% CI, bootstrap over kernels)")
    rows = []
    for m in models:
        for final in (False, True):
            diff = {}
            for k in {c[0]["kernel"] for c in of(m)}:
                a, b = of(m, kernel=k, level="L0"), of(m, kernel=k, level="L3")
                if a and b:
                    diff[k] = rate(b, final) - rate(a, final)
            ks = list(diff)
            rng = random.Random(0)
            boots = sorted(sum(diff[k] for k in rng.choices(ks, k=len(ks))) / len(ks) for _ in range(2000))
            mean = sum(diff.values()) / len(ks)
            rows.append([short(m), "after repair" if final else "first try",
                         f"{100 * mean:+.0f} pts [{100 * boots[50]:+.0f}, {100 * boots[1949]:+.0f}]",
                         sum(d > 0 for d in diff.values()), sum(d == 0 for d in diff.values()), sum(d < 0 for d in diff.values())])
    table(["model", "", "L3 - L0", "kernels better", "same", "worse"], rows)

    heading("Pass rate by level")
    table(["model"] + [f"{l} first / repaired" for l in LEVELS],
          [[short(m)] + [f"{fmt(rate(of(m, level=l), False))} / {fmt(rate(of(m, level=l), True))}" for l in LEVELS] for m in models])

    heading("Why first tries fail (share of failing first tries)")
    reasons = collections.Counter(reason(c[0]) for c in cells if c[0]["stage"] != "pass")
    names = [r for r, _ in reasons.most_common()]
    rows = []
    for m in models:
        fails = collections.Counter(reason(c[0]) for c in of(m) if c[0]["stage"] != "pass")
        total = sum(fails.values())
        rows.append([short(m), total] + [fmt(fails[r] / total) if total else "-" for r in names])
    table(["model", "failing"] + names, rows)

    heading("Kernels (first try / after repair)")
    kernels = sorted({c[0]["kernel"] for c in cells}, key=lambda k: rate([c for c in cells if c[0]["kernel"] == k], False))
    table(["kernel", "source"] + [short(m) for m in models],
          [[k, "PolyBench" if k in POLYBENCH else "other"] +
           [f"{fmt(rate(of(m, kernel=k), False))} / {fmt(rate(of(m, kernel=k), True))}" for m in models] for k in kernels])

    heading("PolyBench-derived vs other kernels (first try / after repair)")
    table(["model", "PolyBench", "other"],
          [[short(m)] + [f"{fmt(rate(g, False))} / {fmt(rate(g, True))} (n={len(g)})"
                         for g in ([c for c in of(m) if c[0]["kernel"] in POLYBENCH],
                                   [c for c in of(m) if c[0]["kernel"] not in POLYBENCH])] for m in models])

    heading("Most common assembler errors (first try)")
    errors = collections.Counter()
    for c in cells:
        if c[0]["stage"] == "assemble_fail":
            e = re.sub(r"^line \d+: (Error: )?", "", c[0].get("first_error") or "")
            errors[short(c[0]["model"]), re.sub(r"(?<!for )`[^`']*['`]", "`X`", e)[:70]] += 1
    table(["count", "model", "error"], [[n, m, e] for (m, e), n in errors.most_common(15)])

    heading("Checks on passing code (final round)")
    rows = []
    for m in models:
        n = calls = suspicious = 0
        for c in of(m):
            last = c[-1]
            path = os.path.join(run_dir, "code", f"{last['cell']}.r{last['round']}.S")
            if last["stage"] != "pass" or not os.path.exists(path):
                continue
            asm = open(path, errors="replace").read()
            sizes = {int(x) for s in last["sizes"] for x in re.findall(r"\d+", s)} - {0, 1, 2, 3, 4, 8}
            immediates = {int(x) for x in re.findall(r"\$(\d+)\b", asm)}
            n += 1
            calls += bool(re.search(r"^\s*callq?\s", asm, re.M))
            suspicious += bool(immediates & (sizes | {8 * s for s in sizes}))
        rows.append([short(m), n, calls, suspicious])
    table(["model", "passing cells", "with call instruction", "immediate equal to a test size (check by hand)"], rows)


if __name__ == "__main__":
    main()
