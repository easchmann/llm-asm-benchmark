#!/usr/bin/env python3
# merge rule and judge results into failures.csv; write summary.md (by run/model/level/kernel category,
# assembler subtypes, repair transitions, desc_full vs desc_refplan) and, if labels.csv exists, agreement.md
# judged attempts come from a sample of whole cells (judge_sample.tsv): they are weighted up to all pending
# attempts of their run, so judged categories are estimates; rule categories are exact
# usage: python3 scripts/ea_tables.py

import collections
import contextlib
import csv
import json
import math
import os

from desc_repair import trajectories
from ea_common import CATEGORIES, OUT, RUNS
from ea_rules import FIELDS
from gen_desc import load_records
from md import heading, table


def cell_of(tag):
    return tag.rsplit(".r", 1)[0]


def merge():
    judged = {}
    if os.path.exists(os.path.join(OUT, "judge.jsonl")):
        for line in open(os.path.join(OUT, "judge.jsonl")):
            r = json.loads(line)
            judged[r["run"], r["tag"]] = r
    sample = {tuple(l.split("\t")) for l in open(os.path.join(OUT, "judge_sample.tsv")).read().splitlines()}
    rows = list(csv.DictReader(open(os.path.join(OUT, "rules.csv"))))
    pending = collections.Counter(r["run"] for r in rows if r["source"] == "pending")
    done = collections.Counter(r for r, t in judged if (r, t) in sample)
    for r in rows:
        r["weight"] = 1.0
        if r["source"] != "pending":
            continue
        j = judged.get((r["run"], r["tag"]))
        r["weight"] = pending[r["run"]] / done[r["run"]] if j and (r["run"], r["tag"]) in sample else 0.0
        if j:
            r.update(category=j["category"], subtype=j["subtype"], evidence=j["evidence"], source="judge",
                     secondary=",".join(sorted(set(filter(None, (r["secondary"] + "," + j["secondary"]).split(","))))))
    with open(os.path.join(OUT, "failures.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, FIELDS + ["weight"])
        w.writeheader()
        w.writerows(rows)
    return rows, sample


def pct(n, total):
    return f"{n:.0f} ({100 * n / total:.0f}%)" if total else "0"


def by(rows, key, title):
    heading(title)
    groups = collections.defaultdict(collections.Counter)
    for r in rows:
        groups[key(r)][r["category"]] += r["weight"]
    cats = [c for c in CATEGORIES if any(g[c] for g in groups.values())]
    table([title.split(" by ")[-1], "n"] + cats,
          [[k, f"{sum(g.values()):.0f}"] + [pct(g[c], sum(g.values())) for c in cats] for k, g in sorted(groups.items())])


def judged_ci(rows, sample):
    heading("Judged categories within the sample of pending attempts (share, 95% interval)")
    print("Pending attempts are the test failures the rules could not classify. Intervals treat attempts as "
          "independent; attempts of one cell are correlated, so they are somewhat too narrow.\n")
    out = []
    for run in RUNS:
        sel = [r for r in rows if r["run"] == run and r["source"] == "judge" and (r["run"], r["tag"]) in sample]
        n = len(sel)
        c = collections.Counter(r["category"] for r in sel)
        row = [run, n]
        for cat in "CDEFG":
            p = c[cat] / n if n else 0
            h = 1.96 * math.sqrt(p * (1 - p) / n) if n else 0
            row.append(f"{100 * p:.0f}% ({100 * max(0, p - h):.0f}-{100 * min(1, p + h):.0f})")
        out.append(row)
    table(["run", "judged", "C", "D", "E", "F", "G"], out)


def subtypes(rows, cat, key, title):
    heading(title)
    groups = collections.defaultdict(collections.Counter)
    for r in rows:
        if r["category"] == cat:
            groups[key(r)][r["subtype"]] += r["weight"]
    subs = sorted({s for g in groups.values() for s in g})
    table(["", "n"] + subs, [[k, f"{sum(g.values()):.0f}"] + [pct(g[s], sum(g.values())) for s in subs]
                             for k, g in sorted(groups.items())])


def transitions(rows, sample):
    heading("Repair transitions: category at attempt k (rows) -> attempt k+1 (columns), weighted cells")
    cat = {(r["run"], r["tag"]): r["category"] for r in rows}
    src = {(r["run"], r["tag"]): r["source"] for r in rows}
    sampled = {(run, cell_of(t)) for run, t in sample}
    open_cells = collections.Counter(run for run, c in {(r["run"], cell_of(r["tag"])) for r in rows if r["source"] in ("pending", "judge")})
    m = collections.defaultdict(collections.Counter)
    for run, run_dir in RUNS.items():
        n_sampled = sum(1 for s in sampled if s[0] == run)
        for cell, rounds in trajectories(load_records(run_dir)).items():
            tags = [f"{cell}.r{r['round']}" for r in rounds]
            has_pending = any(src.get((run, t)) in ("pending", "judge") for t in tags)
            w = (open_cells[run] / n_sampled if (run, cell) in sampled else 0) if has_pending else 1
            if not w:
                continue
            for a, b in zip(rounds, rounds[1:]):
                if a["stage"] in ("pass", "no_answer") or b["stage"] == "no_answer":
                    continue
                to = "pass" if b["stage"] == "pass" else cat[run, f"{cell}.r{b['round']}"]
                frm = cat[run, f"{cell}.r{a['round']}"]
                if frm and to:
                    m[frm][to] += w
    cols = [c for c in CATEGORIES + ["pass"] if any(r[c] for r in m.values())]
    table(["from \\ to", "n"] + cols + ["fixed", "same category"],
          [[k, f"{sum(g.values()):.0f}"] + [f"{g[c]:.0f}" for c in cols] +
           [pct(g["pass"], sum(g.values())), pct(g[k], sum(g.values()))] for k, g in sorted(m.items())])


def plan_effect(rows):
    heading("desc_full vs desc_plan vs desc_refplan on matched cells (same model, kernel, level, repeat)")
    cells = {run: {c for c, rs in trajectories(load_records(d)).items() if rs[0]["stage"] != "no_answer"}
             for run, d in RUNS.items()}
    common = set.intersection(*cells.values())
    stats = {}
    for run in RUNS:
        sel = [r for r in rows if r["run"] == run and cell_of(r["tag"]) in common]
        all_, first = collections.Counter(), collections.Counter()
        for r in sel:
            all_[r["category"]] += r["weight"]
            if r["attempt"] == "0":
                first[r["category"]] += r["weight"]
        stats[run] = (all_, first)
    print(f"{len(common)} matched cells. Failed attempts per 100 cells (all attempts / first attempt only); "
          "C-G are partly estimated from the judged sample.\n")
    per = lambda n: f"{100 * n / len(common):.1f}"
    table(["category"] + list(RUNS) + ["refplan - full"],
          [[c] + [f"{per(stats[r][0][c])} / {per(stats[r][1][c])}" for r in RUNS] +
           [f"{per(stats['desc_refplan'][0][c] - stats['desc_full'][0][c])} / "
            f"{per(stats['desc_refplan'][1][c] - stats['desc_full'][1][c])}"]
           for c in CATEGORIES if any(stats[r][0][c] for r in RUNS)])


def kappa(pairs):
    n = len(pairs)
    if not n:
        return float("nan")
    po = sum(a == b for a, b in pairs) / n
    ca, cb = collections.Counter(a for a, _ in pairs), collections.Counter(b for _, b in pairs)
    pe = sum(ca[k] * cb[k] for k in ca) / n / n
    return (po - pe) / (1 - pe) if pe < 1 else 1.0


def agreement(rows):
    path = os.path.join(OUT, "labels.csv")
    if not os.path.exists(path):
        return
    auto = {(r["run"], r["tag"]): r for r in rows}
    pairs = [(l["category"], auto[l["run"], l["tag"]]["category"], auto[l["run"], l["tag"]]["source"])
             for l in csv.DictReader(open(path))]
    with open(os.path.join(OUT, "agreement.md"), "w") as f, contextlib.redirect_stdout(f):
        heading(f"Agreement with hand labels ({len(pairs)} attempts)")
        print(f"Overall Cohen's kappa: {kappa([(h, a) for h, a, _ in pairs]):.2f}, "
              f"raw agreement {sum(h == a for h, a, _ in pairs) / len(pairs):.0%}\n")
        for src in ("rule", "judge"):
            sub = [(h, a) for h, a, s in pairs if s == src]
            if sub:
                print(f"- {src}: n={len(sub)}, kappa {kappa(sub):.2f}, agreement {sum(h == a for h, a in sub) / len(sub):.0%}")
        print()
        cats = [c for c in CATEGORIES if any(c in (h, a) for h, a, _ in pairs)]
        table(["category", "hand n", "classifier n", "kappa (one vs rest)"],
              [[c, sum(h == c for h, _, _ in pairs), sum(a == c for _, a, _ in pairs),
                f"{kappa([(h == c, a == c) for h, a, _ in pairs]):.2f}"] for c in cats])
        heading("Confusion (rows: hand label, columns: classifier)")
        table(["hand \\ classifier"] + cats, [[h] + [sum(x == h and y == a for x, y, _ in pairs) for a in cats] for h in cats])
    print(open(os.path.join(OUT, "agreement.md")).read())


def main():
    rows, sample = merge()
    known = [r for r in rows if r["weight"]]
    with open(os.path.join(OUT, "summary.md"), "w") as f, contextlib.redirect_stdout(f):
        print("# Failure classification\n\nOne main category per failed attempt; see prompts/error_rubric.md. "
              "A, B, X and the rule-based C/D/E are exact counts. Test failures the rules could not classify were "
              "judged for a random sample of whole cells (judge_sample.tsv) and weighted up to their run, so the "
              "remaining C-G counts are estimates. Cells: count (share of the row's failed attempts).\n")
        by(known, lambda r: r["run"], "Main category by run")
        judged_ci(rows, sample)
        by(known, lambda r: (r["run"], r["model"]), "Main category by run and model")
        by(known, lambda r: (r["run"], r["level"]), "Main category by run and level")
        by(known, lambda r: (r["run"], r["kernel_category"]), "Main category by run and kernel category")
        by(known, lambda r: r["source"], "Main category by source")
        subtypes(known, "A", lambda r: (r["run"], r["model"]), "Assembler errors (A) by subtype, run and model")
        for c in "CDEF":
            subtypes(known, c, lambda r: r["run"], f"{c} by subtype and run")
        transitions(rows, sample)
        plan_effect(known)
    print(open(os.path.join(OUT, "summary.md")).read()[:2500])
    agreement(rows)


if __name__ == "__main__":
    main()
