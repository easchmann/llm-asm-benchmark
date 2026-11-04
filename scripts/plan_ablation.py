#!/usr/bin/env python3
# plan variants vs no plan (desc_full) and teacher plan (desc_refplan) on the same cells (model, kernel, level, repeat):
# first-try and after-repair pass rates, paired differences with 95% CI and exact McNemar p, overall and by model,
# level and kernel category; for seeded_error runs, whether answers reproduce the injected error
# usage: python3 scripts/plan_ablation.py results/desc_full results/desc_refplan results/desc_plan_algo [...]
#        python3 scripts/plan_ablation.py --classify-seeded results/desc_plan_seeded    (on riken: builds and runs
#        every answer against plans/seeded_error/<kernel>/seeded_ref.c, writes <run>/seeded_outcomes.json)

import collections
import json
import math
import os
import subprocess
import sys
import tempfile

from desc_compare import answered, short
from desc_repair import CATEGORY

LEVELS = ["L0", "L1", "L2", "L3"]
OUTCOMES = ["fixed", "reproduces", "fails differently"]


def heading(text):
    print(f"## {text}\n")


def table(header, rows):
    print("| " + " | ".join(map(str, header)) + " |\n|" + "---|" * len(header))
    for row in rows:
        print("| " + " | ".join(map(str, row)) + " |")
    print()


def passed(cell, final):
    return (cell[-1] if final else cell[0])["stage"] == "pass"


def mcnemar(b, c):
    n = b + c
    if n == 0:
        return 1.0
    return min(1.0, 2 * sum(math.comb(n, i) for i in range(min(b, c) + 1)) / 2 ** n)


def paired(base, new, final):
    b = sum(passed(x, final) and not passed(y, final) for x, y in zip(base, new))
    c = sum(passed(y, final) and not passed(x, final) for x, y in zip(base, new))
    n = len(base)
    d = (c - b) / n
    se = math.sqrt(max(b + c - (c - b) ** 2 / n, 0)) / n
    p = mcnemar(b, c)
    return f"{100 * d:+.1f} [{100 * (d - 1.96 * se):+.1f}, {100 * (d + 1.96 * se):+.1f}] p={p:.2g}" + ("*" if p < 0.05 else "")


def rate(cells, final):
    return f"{100 * sum(passed(c, final) for c in cells) / len(cells):.1f}%"


def groups(keys):
    out = [("all", keys)]
    for name, f in (("model", lambda k: short(k[0])), ("level", lambda k: k[2]), ("category", lambda k: CATEGORY[k[1]])):
        for value in sorted({f(k) for k in keys}):
            out.append((f"{name}: {value}", [k for k in keys if f(k) == value]))
    return out


def compare(full, ref, runs, name):
    var = runs[name]
    keys = sorted(full.keys() & ref.keys() & var.keys())
    heading(f"{name}: {len(keys)} cells answered in all three runs (B = desc_full, R = desc_refplan, V = {name})")
    rows = []
    for label, ks in groups(keys):
        row = [label, len(ks)]
        for final in (False, True):
            b, r, v = ([run[k] for k in ks] for run in (full, ref, var))
            row += [rate(b, final), rate(r, final), rate(v, final), paired(b, v, final), paired(r, v, final)]
        rows.append(row)
    table(["group", "cells", "first B", "first R", "first V", "first V - B", "first V - R",
           "repair B", "repair R", "repair V", "repair V - B", "repair V - R"], rows)


def seeded_outcome(asm, kernel, work):
    from gen_desc import DESC, ROOT, run
    exe = os.path.join(work, "a")
    kdir = os.path.join(DESC, kernel)
    for cmd in (["gcc", "-c", asm, "-o", exe + ".o"],
                ["gcc", "-O2", "-I", os.path.join(DESC, "common"), os.path.join(kdir, "driver.c"),
                 os.path.join(ROOT, "plans", "seeded_error", kernel, "seeded_ref.c"), exe + ".o", "-o", exe, "-lm"]):
        if subprocess.run(cmd, capture_output=True).returncode:
            return "fails differently"
    return "reproduces" if run(exe)[0] == "pass" else "fails differently"


def classify_seeded(run_dir):
    from desc_repair import trajectories
    from gen_desc import load_records
    out, work = {}, tempfile.mkdtemp(prefix="seeded_")
    for cell, rounds in trajectories(load_records(run_dir)).items():
        if rounds[0]["stage"] == "no_answer":
            continue
        result = []
        for r in (rounds[0], rounds[-1]):
            asm = os.path.join(run_dir, "code", f"{cell}.r{r['round']}.S")
            if r["stage"] == "pass":
                result.append("fixed")
            else:
                result.append(seeded_outcome(asm, r["kernel"], work) if os.path.exists(asm) else "fails differently")
        out[cell] = result
    path = os.path.join(run_dir, "seeded_outcomes.json")
    json.dump(out, open(path, "w"), indent=0)
    print(f"wrote {len(out)} cells to {path}")


def seeded_report(run_dir, runs, ref):
    path = os.path.join(run_dir, "seeded_outcomes.json")
    if not os.path.exists(path):
        print(f"(no {path}; run --classify-seeded on riken first)\n")
        return
    outcomes = json.load(open(path))
    cells = {k: c for k, c in runs[os.path.basename(os.path.normpath(run_dir))].items() if c[0]["cell"] in outcomes}
    heading("seeded_error: fixed (passes the real tests) / reproduces the injected error / fails differently")
    rows = []
    for label, ks in groups(sorted(cells)) + [(f"kernel: {k}", [x for x in cells if x[1] == k]) for k in sorted({x[1] for x in cells})] \
            + [("teacher plan passed first try", [k for k in cells if k in ref and passed(ref[k], False)])]:
        if not ks:
            continue
        row = [label, len(ks)]
        for i in (0, 1):
            mix = collections.Counter(outcomes[cells[k][0]["cell"]][i] for k in ks)
            row += [f"{100 * mix[o] / len(ks):.0f}%" for o in OUTCOMES]
        rows.append(row)
    table(["group", "cells"] + [f"{when}: {o}" for when in ("first", "repair") for o in OUTCOMES], rows)


def main():
    if sys.argv[1] == "--classify-seeded":
        classify_seeded(sys.argv[2])
        return
    full_dir, ref_dir, *variant_dirs = sys.argv[1:]
    name = lambda d: os.path.basename(os.path.normpath(d))
    full, ref = answered(full_dir), answered(ref_dir)
    runs = {name(d): answered(d) for d in variant_dirs}
    print("Differences in points, 95% CI (paired Wald), exact McNemar p (* p < 0.05). Cells are paired on model, kernel,\n"
          "level and repeat; repeats and kernels are not independent, so the CIs are optimistic.\n")
    for d in variant_dirs:
        compare(full, ref, runs, name(d))
        first = next(iter(runs[name(d)].values()), [{}])[0]
        if "seeded_error" in (first.get("plan_source") or ""):
            seeded_report(d, runs, ref)


if __name__ == "__main__":
    main()
