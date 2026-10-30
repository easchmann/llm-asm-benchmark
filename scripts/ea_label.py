#!/usr/bin/env python3
# hand labels for validating the classifier
# usage: python3 scripts/ea_label.py sample [--n 100]   draw a sample stratified by classifier category
#        python3 scripts/ea_label.py label                label it (resumable; the classifier's answer is not shown)

import argparse
import collections
import csv
import json
import os
import random
import subprocess
import tempfile

from ea_common import CATEGORIES, OUT, RUNS
from ea_judge import CATS, evidence

SAMPLE = os.path.join(OUT, "validation_sample.csv")
LABELS = os.path.join(OUT, "labels.csv")


def sample(n, seed=1):
    rows = list(csv.DictReader(open(os.path.join(OUT, "failures.csv"))))
    by_cat = collections.defaultdict(list)
    for r in rows:
        by_cat[r["category"]].append(r)
    floor = 6
    share = {c: max(floor, round(n * len(v) / len(rows))) for c, v in by_cat.items()}
    while sum(share.values()) > n:
        share[max(share, key=lambda c: share[c] - len(by_cat[c]) / len(rows) * n)] -= 1
    rng = random.Random(seed)
    picked = []
    for c, k in share.items():
        strata = collections.defaultdict(list)
        for r in by_cat[c]:
            strata[r["run"], r["model"]].append(r)
        pools = [rng.sample(v, len(v)) for v in strata.values()]
        while len([p for p in picked if p["category"] == c]) < min(k, len(by_cat[c])):
            for pool in pools:
                if pool and len([p for p in picked if p["category"] == c]) < k:
                    picked.append(pool.pop())
    rng.shuffle(picked)
    with open(SAMPLE, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["run", "tag"])
        w.writerows([r["run"], r["tag"]] for r in picked)
    print(f"{len(picked)} attempts -> {SAMPLE}", dict(collections.Counter(r["category"] for r in picked)))


def show(row):
    if row["stage"] in ("wrong", "crash", "timeout"):
        text = evidence(row)
    else:
        rt = os.path.join(OUT, "retest", row["run"], row["tag"] + ".json")
        build = json.load(open(rt)).get("build", "") if os.path.exists(rt) else ""
        asm = os.path.join(RUNS[row["run"]], "code", row["tag"] + ".S")
        lines = open(asm, errors="replace").read().splitlines() if os.path.exists(asm) else []
        text = f"stage: {row['stage']}\n\n{build}\n\n" + "\n".join(f"{i:4d} | {l}" for i, l in enumerate(lines, 1))
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False) as f:
        f.write(f"{row['run']}  {row['tag']}  (stage {row['stage']})\n\n{text}\n")
    subprocess.run(["less", "-R", f.name])
    os.remove(f.name)


def ask(text, ok):
    while True:
        v = input(text).strip()
        if v in ok:
            return v


def label():
    full = {(r["run"], r["tag"]): r for r in csv.DictReader(open(os.path.join(OUT, "failures.csv")))}
    todo = [full[r["run"], r["tag"]] for r in csv.DictReader(open(SAMPLE))]
    done = {(r["run"], r["tag"]) for r in csv.DictReader(open(LABELS))} if os.path.exists(LABELS) else set()
    new = not os.path.exists(LABELS)
    with open(LABELS, "a", newline="") as f:
        w = csv.writer(f)
        if new:
            w.writerow(["run", "tag", "category", "subtype", "secondary", "note"])
        for i, row in enumerate(todo, 1):
            if (row["run"], row["tag"]) in done:
                continue
            show(row)
            print(f"\n[{i}/{len(todo)}] {row['tag']} ({row['stage']})   v = view again, q = quit")
            cat = ask(f"category {'/'.join(CATEGORIES)}: ", set(CATEGORIES) | {"v", "q"})
            while cat == "v":
                show(row)
                cat = ask("category: ", set(CATEGORIES) | {"q"})
            if cat == "q":
                return
            subs = sorted(CATS.get(cat, ())) + (["no_code", "incomplete"] if cat == "X" else [])
            print("  " + "  ".join(f"{j}:{s}" for j, s in enumerate(subs)))
            sub = ask("subtype number (enter = none): ", {str(j) for j in range(len(subs))} | {""})
            sec = input("secondary categories (e.g. EF, enter = none): ").strip().upper()
            note = input("note: ").strip()
            w.writerow([row["run"], row["tag"], cat, subs[int(sub)] if sub else "", ",".join(sorted(set(sec) & set(CATEGORIES))), note])
            f.flush()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("cmd", choices=["sample", "label"])
    p.add_argument("--n", type=int, default=100)
    args = p.parse_args()
    sample(args.n) if args.cmd == "sample" else label()


if __name__ == "__main__":
    main()
