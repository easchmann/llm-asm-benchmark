#!/usr/bin/env python3
# the "via C" baseline next to the assembly run: C-only failure stages, compiler errors, per-kernel pass rates,
# what still fails, and an offline re-score of answers whose code block was never closed
# (the extractor then found no block and handed the opening ``` line to the assembler or compiler)
# usage: python3 scripts/desc_viac.py results/desc_full results/desc_viac
# the re-score reads results/<run>/raw and rebuilds and tests the answers; it makes no API calls

import collections
import os
import re
import sys
import tempfile

from desc_compare import LEVELS, answered, diff, pct, rate, short
from gen_desc import DESC, check, extract_asm, extract_c, load_kernel
from md import heading, table

FENCE = "```"


def unclosed(r, run_dir):
    if r.get("extract_reason") != "unfenced":
        return None
    path = os.path.join(run_dir, "raw", f"{r['cell']}.r{r['round']}.txt")
    reply = open(path).read() if os.path.exists(path) else ""
    return reply if reply.count(FENCE) % 2 else None


def rescore(r, reply, tmp):
    _, spec, _ = load_kernel(r["kernel"], r["level"])
    c = r.get("mode") == "c"
    code, _ = (extract_c if c else extract_asm)(reply + "\n" + FENCE, spec["name"])
    if code is None:
        return r["stage"]
    path = os.path.join(tmp, r["cell"] + (".c" if c else ".S"))
    open(path, "w").write(code)
    return check(path, os.path.join(DESC, r["kernel"]), os.path.join(tmp, r["cell"]))[0]


def rescored(cells, run_dir, tmp):
    fixed, changes = {}, []
    for key, rounds in cells.items():
        reply = unclosed(rounds[0], run_dir)
        stage = rescore(rounds[0], reply, tmp) if reply else rounds[0]["stage"]
        if reply:
            changes.append((rounds[0], stage))
        fixed[key] = [dict(rounds[0], stage=stage)] if stage == "pass" else rounds
    return fixed, changes


def pass_table(pairs, groups):
    return [[m, l, len(g)] + [x for final in (False, True)
                              for x in (f"{100 * rate(g, 0, final):.0f}%", f"{100 * rate(g, 1, final):.0f}%", diff(g, final))]
            for m, l, g in groups(pairs) if g]


def by_model(pairs):
    models = sorted({p[0][0]["model"] for p in pairs})
    return [("all", "all", pairs)] + [(short(m), "all", [p for p in pairs if p[0][0]["model"] == m]) for m in models]


def main():
    dirs = sys.argv[1:3]
    names = [os.path.basename(os.path.normpath(d)) for d in dirs]
    a, b = map(answered, dirs)
    keys = sorted(a.keys() & b.keys())
    models = sorted({k[0] for k in keys})
    pass_header = ["model", "level", "cells", f"first try {names[0]}", f"first try {names[1]}", "C - asm",
                   f"after repair {names[0]}", f"after repair {names[1]}", "C - asm"]

    heading("Failure stages that exist only for C (share of cells, first try / any round)")
    rows = []
    for m in models:
        cells = [b[k] for k in keys if k[0] == m]
        records = [r for c in cells for r in c]
        for stage in ("forbidden", "compile_fail"):
            rows.append([short(m), stage, pct(sum(c[0]["stage"] == stage for c in cells), len(cells)),
                         sum(r["stage"] == stage for r in records)])
    table(["model", "stage", "first try", "records, all rounds"], rows)

    heading(f"Compiler errors in {names[1]}, first error line (all rounds)")
    rows = []
    for m in models:
        records = [r for k in keys if k[0] == m for r in b[k] if r["stage"] == "compile_fail"]
        kinds = collections.Counter(re.sub(r"^line \d+: ", "", r.get("first_error") or "-")[:70] for r in records)
        rows += [[short(m), n, kind] for kind, n in kinds.most_common(3)]
    table(["model", "records", "first error"], rows)

    with tempfile.TemporaryDirectory() as tmp:
        fixed, changes = zip(*(rescored({k: cells[k] for k in keys}, d, tmp) for cells, d in ((a, dirs[0]), (b, dirs[1]))))
    heading("Answers with an unclosed code block, re-scored offline with the block closed (first try)")
    rows = []
    for name, ch in zip(names, changes):
        for m in models:
            mine = [(r, s) for r, s in ch if r["model"] == m]
            if mine:
                after = collections.Counter(s for _, s in mine)
                rows.append([name, short(m), len(mine), collections.Counter(r["stage"] for r, _ in mine).most_common(1)[0][0],
                             ", ".join(f"{s} {n}" for s, n in after.most_common())])
    table(["run", "model", "first tries", "recorded as", "re-scored"], rows)

    heading("Pass rate, C - asm, as recorded (95% CI, bootstrap over kernels)")
    table(pass_header, pass_table([(a[k], b[k]) for k in keys], by_model))
    heading("Pass rate, C - asm, unclosed code blocks re-scored (95% CI, bootstrap over kernels)")
    table(pass_header, pass_table([(fixed[0][k], fixed[1][k]) for k in keys], by_model))

    heading("Per kernel, all models (unclosed code blocks re-scored)")
    rows = []
    for kernel in sorted({k[1] for k in keys}):
        g = [(fixed[0][k], fixed[1][k]) for k in keys if k[1] == kernel]
        rows.append([kernel, len(g)] + [f"{100 * rate(g, side, final):.0f}%" for final in (False, True) for side in (0, 1)])
    rows.sort(key=lambda row: float(row[2][:-1]))
    table(["kernel", "cells", f"first try {names[0]}", f"first try {names[1]}",
           f"after repair {names[0]}", f"after repair {names[1]}"], rows)

    heading("Cells that fail after repair (unclosed code blocks re-scored)")
    rows = []
    for m in models:
        for side, name in enumerate(names):
            failed = [k for k in keys if k[0] == m and fixed[side][k][-1]["stage"] != "pass"]
            kernels = collections.Counter(k[1] for k in failed)
            stages = collections.Counter(fixed[side][k][-1]["stage"] for k in failed)
            rows.append([short(m), name, len(failed), ", ".join(f"{k} {n}" for k, n in kernels.most_common(5)) or "-",
                         ", ".join(f"{s} {n}" for s, n in stages.most_common()) or "-"])
    only_asm = [k for k in keys if fixed[0][k][-1]["stage"] == "pass" and fixed[1][k][-1]["stage"] != "pass"]
    table(["model", "run", "cells", "kernels (top 5)", "final stage"], rows)
    print(f"Cells solved in {names[0]} but not in {names[1]}: {len(only_asm)}"
          + (" (" + ", ".join(f"{short(k[0])} {k[1]} {k[2]} rep{k[3]}" for k in only_asm) + ")" if only_asm else "") + "\n")

    heading("Per level, all models (unclosed code blocks re-scored)")
    table(pass_header, pass_table([(fixed[0][k], fixed[1][k]) for k in keys],
                                  lambda ps: [("all", l, [p for p in ps if p[0][0]["level"] == l]) for l in LEVELS]))


if __name__ == "__main__":
    main()
