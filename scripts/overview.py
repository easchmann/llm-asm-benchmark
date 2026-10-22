#!/usr/bin/env python3
# overview tables of a description->asm run, as markdown
# usage: python3 scripts/overview.py results/desc_full

import collections
import sys

from desc_repair import C_STAGES, CATEGORY, ORDER, trajectories
from gen_desc import load_records
from md import heading, table


def pct(k, n):
    return f"{100 * k / n:.0f}%" if n else "-"


def summary(cells):
    answered = [rs for rs in cells if rs[0]["stage"] != "no_answer"]
    first = sum(rs[0]["stage"] == "pass" for rs in answered)
    final = sum(rs[-1]["stage"] == "pass" for rs in answered)
    cost = sum(r.get("cost_usd") or 0 for rs in cells for r in rs)
    return [len(cells), len(cells) - len(answered), pct(first, len(answered)), pct(final, len(answered)), f"${cost:.2f}"]


cells = list(trajectories(load_records(sys.argv[1])).values())
groups = collections.defaultdict(list)
for rs in cells:
    r = rs[0]
    groups["all", r["model"]].append(rs)
    groups["level", r["model"], r["level"]].append(rs)
    groups["category", r["model"], CATEGORY.get(r["kernel"], "other")].append(rs)

heading("Per model")
table(["model", "cells", "no answer", "pass first try", "pass after repair", "cost"],
            [[k[1]] + summary(v) for k, v in sorted(groups.items()) if k[0] == "all"])

for name in ("level", "category"):
    heading(f"Per model and {name}")
    table(["model", name, "cells", "no answer", "pass first try", "pass after repair", "cost"],
                [[k[1], k[2]] + summary(v) for k, v in sorted(groups.items()) if k[0] == name])

used = {rs[-1]["stage"] for rs in cells}
stages = [s for s in ORDER if s != "pass" and (s not in C_STAGES or s in used)]
rows = []
for k, v in sorted(groups.items()):
    if k[0] == "all":
        final = collections.Counter(rs[-1]["stage"] for rs in v)
        rows.append([k[1]] + [final[s] for s in stages])
heading("Final stage of cells that did not pass")
table(["model"] + stages, rows)
