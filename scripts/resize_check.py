#!/usr/bin/env python3
# re-run the x86 passes at other PolyBench dataset sizes
# usage: python3 scripts/resize_check.py [run_dir ...]

import csv
import glob
import os
import subprocess
import sys

from llm_runner import grade_outputs, run_binary

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SUITE = os.path.join(ROOT, "kernels", "polybench-c-4.2")
UTIL = os.path.join(SUITE, "utilities")
WORK = os.path.join(ROOT, ".work", "resize_check")
OUT_CSV = os.path.join(ROOT, "results", "resize_check.csv")
SIZES = ["MINI", "SMALL", "MEDIUM", "LARGE"]


def kernel_dir(kernel):
    for d, _, files in os.walk(SUITE):
        if f"{kernel}.c" in files:
            return d
    sys.exit(f"kernel {kernel} not found")


def build(src, kdir, size, exe):
    cmd = ["gcc", "-O0", f"-D{size}_DATASET", "-DPOLYBENCH_DUMP_ARRAYS",
           "-I", UTIL, "-I", kdir, os.path.join(UTIL, "polybench.c"), src, "-o", exe, "-lm"]
    return subprocess.run(cmd, capture_output=True).returncode == 0


def run(exe):
    out = exe + ".txt"
    rc, timed_out = run_binary(exe, out, exe + ".err.log")
    if timed_out:
        return None, "timeout"
    if rc != 0:
        return None, "run_fail"
    return out, None


def native_output(kernel, kdir, size):
    exe = os.path.join(WORK, f"{kernel}_native_{size}")
    if not os.path.exists(exe + ".txt"):
        src = os.path.join(kdir, f"{kernel}.c")
        if not build(src, kdir, size, exe):
            sys.exit(f"native build failed: {kernel} {size}")
        run(exe)
    return exe + ".txt"


def check(kernel, src, tag, size):
    kdir = kernel_dir(kernel)
    native = native_output(kernel, kdir, size)
    exe = os.path.join(WORK, f"{tag}_{size}")
    if not build(src, kdir, size, exe):
        return "build_fail"
    out, err = run(exe)
    if err:
        return err
    g = grade_outputs(native, out)
    return g["result"] if g["result"] == "pass" else f"{g['result']} {g['right']}/{g['total']}"


def main():
    runs = sys.argv[1:] or sorted(glob.glob(os.path.join(ROOT, "results", "x86_*_repeats", "x86_*_r?")))
    os.makedirs(WORK, exist_ok=True)
    rows = []
    for run_dir in runs:
        for r in csv.DictReader(open(os.path.join(run_dir, "comparison.csv"))):
            if r["result"] != "pass":
                continue
            slug = r["alias"].replace("/", "_")
            src = os.path.join(run_dir, "code", f"{r['kernel']}__{slug}.c")
            tag = f"{os.path.basename(run_dir)}__{r['kernel']}__{slug}"
            row = {"run": os.path.basename(run_dir), "kernel": r["kernel"], "model": r["alias"]}
            for size in SIZES:
                row[size] = check(r["kernel"], src, tag, size)
            print(row["run"], row["kernel"], row["model"], *(row[s] for s in SIZES))
            rows.append(row)

    with open(OUT_CSV, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["run", "kernel", "model"] + SIZES)
        w.writeheader()
        w.writerows(rows)

    print(f"\n{len(rows)} passes checked, written to {OUT_CSV}")
    for size in SIZES:
        print(f"{size:<7} {sum(r[size] == 'pass' for r in rows)}/{len(rows)}")


if __name__ == "__main__":
    main()
