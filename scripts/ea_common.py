# shared helpers for the failure classification in results/error_analysis

import json
import os

from desc_repair import CATEGORY, trajectories
from gen_desc import DESC, ROOT, load_records

HOME = os.path.expanduser("~")
RUNS = {
    "desc_full": os.path.join(HOME, "llm-asm-benchmark", "results", "desc_full"),
    "desc_plan": os.path.join(ROOT, "results", "desc_plan"),
    "desc_refplan": os.path.join(ROOT, "results", "desc_refplan"),
}
OUT = os.path.join(ROOT, "results", "error_analysis")
CATEGORIES = ["A", "B", "C", "D", "E", "F", "G", "X"]


def attempts(runs=RUNS):
    for run, run_dir in runs.items():
        for cell, rounds in sorted(trajectories(load_records(run_dir)).items()):
            for r in rounds:
                if r["stage"] in ("pass", "no_answer"):
                    continue
                tag = f"{cell}.r{r['round']}"
                yield {"run": run, "run_dir": run_dir, "cell": cell, "tag": tag, "model": r["model"],
                       "kernel": r["kernel"], "kernel_category": CATEGORY[r["kernel"]], "level": r["level"],
                       "repeat": r["repeat"], "attempt": r["round"], "stage": r["stage"], "rec": r,
                       "asm": os.path.join(run_dir, "code", tag + ".S")}


def retest_path(a):
    return os.path.join(OUT, "retest", a["run"], a["tag"] + ".json")


def load_retest(a):
    path = retest_path(a)
    return json.load(open(path)) if os.path.exists(path) else {}


def kernel_dir(kernel, sweep=None):
    return os.path.join(OUT, "drivers", f"{kernel}_{sweep}") if sweep else os.path.join(DESC, kernel)
