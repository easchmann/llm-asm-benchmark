#!/usr/bin/env python3
# LLM judge for the attempts the rule pass left pending; appends one record per attempt to judge.jsonl
# usage: RIVAULT_API_KEY=... python3 scripts/ea_judge.py --model <model> [--jobs 8] [--tags file]
#        python3 scripts/ea_judge.py --draw-sample 250  sample whole cells per run until ~250 pending attempts each
#        python3 scripts/ea_judge.py --show 8         print the next unjudged attempts of the sample
#        python3 scripts/ea_judge.py --model <m> --record < verdicts.jsonl   validate and append hand-made verdicts
# resumable: attempts already in judge.jsonl are skipped

import argparse
import csv
import json
import os
import random
import re
import sys
import threading
from concurrent.futures import ThreadPoolExecutor

import llm_client
from ea_common import OUT, RUNS, kernel_dir
from gen_desc import ROOT, THINK_RE

RUBRIC = open(os.path.join(ROOT, "prompts", "error_rubric.md")).read()
CATS = {c: set(re.findall(r"^- (\w+)", block, re.M))
        for c, block in re.findall(r"\*\*([A-GX])\. .*?\n(.*?)(?=\n\*\*|\n## )", RUBRIC, re.S)}
CATS["G"] = {"crash_unknown", "timeout_unknown"}
JSON_RE = re.compile(r"\{[^{}]*\"category\"[^{}]*\}", re.S)
LOG = os.path.join(OUT, "judge.jsonl")
SAMPLE = os.path.join(OUT, "judge_sample.tsv")
lock = threading.Lock()


def read(path, limit=None):
    text = open(path, errors="replace").read() if os.path.exists(path) else ""
    return text if limit is None or len(text) <= limit else text[:limit] + "\n[... truncated]"


def evidence(row):
    run_dir = RUNS[row["run"]]
    kdir = kernel_dir(row["kernel"])
    spec = json.load(open(os.path.join(kdir, "spec.json")))
    asm = read(os.path.join(run_dir, "code", row["tag"] + ".S")).splitlines()[:500]
    rt_path = os.path.join(OUT, "retest", row["run"], row["tag"] + ".json")
    rt = json.load(open(rt_path)) if os.path.exists(rt_path) else {}
    parts = [f"Function: {spec['signature']}", f"Memory layout: {spec['layout']}",
             "Task description given to the model:\n" + read(os.path.join(kdir, row["level"] + ".txt")).strip(),
             "Reference C:\n" + read(os.path.join(kdir, "ref.c")).strip(),
             "Failing assembly (line numbers added):\n" + "\n".join(f"{i:4d} | {l}" for i, l in enumerate(asm, 1)),
             f"Original test result: {row['stage']}\n" + read(os.path.join(run_dir, "code", row["tag"] + ".out"), 3000)]
    for name, title in [("orig", "Re-run at the original sizes with diagnostics"),
                        ("mult", "Size sweep, every dimension a multiple of 8"),
                        ("tail", "Size sweep, sizes not multiples of 8 and small sizes")]:
        r = rt.get(name)
        if r:
            sizes = ", ".join(f"{s} {v}" for s, v in r["sizes"].items()) or "none completed"
            parts.append(f"{title}: {r['stage']}" + (f" ({r['signal']})" if r["signal"] else "") +
                         f"\nsizes: {sizes}\n" + "\n".join(r["fails"][:10]) + ("\n" + r["crash_text"] if r["crash_text"] else ""))
    return "\n\n".join(parts)


def prompt(row):
    return (RUBRIC + "\n\n# Attempt to classify\n\nThis attempt assembled and linked but failed the tests. "
            "Classify it into one of C, D, E, F, G following the rubric.\n\n" + evidence(row) +
            '\n\nAnswer with one JSON object and nothing after it: {"category": "<letter>", "subtype": "<subtype>", '
            '"secondary": "<letters of secondary categories, comma separated, or empty>", '
            '"evidence": "<one or two sentences citing line numbers, instructions and failing sizes>"}')


def parse(text):
    m = JSON_RE.findall(THINK_RE.sub("", text))
    if not m:
        return None
    try:
        d = json.loads(m[-1])
    except json.JSONDecodeError:
        return None
    if d.get("category") not in "CDEFG" or d.get("subtype") not in CATS.get(d.get("category"), ()):
        return None
    d["secondary"] = ",".join(sorted(set(re.findall(r"[A-G]", str(d.get("secondary", "")))) - {d["category"]}))
    return d


def append(rec):
    with lock, open(LOG, "a") as f:
        f.write(json.dumps(rec) + "\n")


def judge(row, model):
    p = prompt(row)
    for _ in range(3):
        try:
            r = llm_client.call_llm(model, p, temperature=0)
        except llm_client.LlmError as e:
            last = str(e)
            continue
        d = parse(r["content"])
        if d:
            rec = {"run": row["run"], "tag": row["tag"], "judge_model": model, **d,
                   "usage": r["usage"], "cost_usd": r["cost_usd"]}
            append(rec)
            return
        last = r["content"][-300:]
    print(f"{row['run']} {row['tag']}: no valid answer ({last})", flush=True)


def draw_sample(rows, n):
    cells = {}
    for r in rows:
        cells.setdefault((r["run"], r["tag"].rsplit(".r", 1)[0]), []).append(r)
    keys = sorted(cells)
    random.Random(1).shuffle(keys)
    picked, count = [], {}
    for run, cell in keys:
        if count.get(run, 0) < n:
            picked += sorted(cells[run, cell], key=lambda r: int(r["attempt"]))
            count[run] = count.get(run, 0) + len(cells[run, cell])
    with open(SAMPLE, "w") as f:
        f.writelines(f"{r['run']}\t{r['tag']}\n" for r in picked)
    print(f"{len(picked)} attempts in {len({(r['run'], r['tag'].rsplit('.r', 1)[0]) for r in picked})} cells", count)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--model")
    p.add_argument("--show", type=int, help="print the evidence for the next N unjudged attempts")
    p.add_argument("--record", action="store_true", help="read verdict JSON lines (with run and tag) from stdin")
    p.add_argument("--draw-sample", type=int, metavar="N", help="draw whole cells per run until N pending attempts")
    p.add_argument("--jobs", type=int, default=8)
    p.add_argument("--tags", help="file with run<TAB>tag lines: judge only these")
    args = p.parse_args()
    done = {(r["run"], r["tag"]) for r in map(json.loads, open(LOG))} if os.path.exists(LOG) else set()
    rows = [r for r in csv.DictReader(open(os.path.join(OUT, "rules.csv"))) if r["source"] == "pending"]
    if args.tags:
        want = {tuple(l.split("\t")) for l in open(args.tags).read().splitlines()}
        rows = [r for r in rows if (r["run"], r["tag"]) in want]
    todo = [r for r in rows if (r["run"], r["tag"]) not in done]
    if args.draw_sample:
        return draw_sample(rows, args.draw_sample)
    if args.show is not None or args.record:
        order = [tuple(l.split("\t")) for l in open(SAMPLE).read().splitlines()]
        pos = {k: i for i, k in enumerate(order)}
        todo = sorted((r for r in todo if (r["run"], r["tag"]) in pos), key=lambda r: pos[r["run"], r["tag"]])
    if args.show is not None:
        print(f"{len(todo)} left")
        for r in todo[:args.show]:
            print(f"\n======== {r['run']}\t{r['tag']}\n" + evidence(r))
        return
    if args.record:
        pending = {(r["run"], r["tag"]) for r in todo}
        for line in sys.stdin.read().splitlines():
            v = json.loads(line)
            d = parse(json.dumps({k: v.get(k, "") for k in ("category", "subtype", "secondary", "evidence")}))
            if (v["run"], v["tag"]) not in pending or not d:
                sys.exit(f"rejected: {line}")
            append({"run": v["run"], "tag": v["tag"], "judge_model": args.model, **d})
        return
    if not args.model:
        p.error("--model is required")
    print(f"{len(todo)} attempts to judge with {args.model}", flush=True)
    with ThreadPoolExecutor(args.jobs) as pool:
        for i, _ in enumerate(pool.map(lambda r: judge(r, args.model), todo), 1):
            if i % 100 == 0:
                print(f"{i}/{len(todo)}", flush=True)


if __name__ == "__main__":
    main()
