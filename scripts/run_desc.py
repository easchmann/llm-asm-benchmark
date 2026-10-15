#!/usr/bin/env python3
# run a matrix of description->asm cells, skipping cells that already finished
# usage: python3 scripts/run_desc.py --run-id desc_pilot --models Qwen/Qwen3.8-27B zai-org/GLM-5.3-Flash \
#            --kernels axpy atax --levels L0 L2 L3 --repeats 2

import argparse
import json
import os
import subprocess
import sys
import time

from gen_desc import cell_id

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GEN = os.path.join(ROOT, "scripts", "gen_desc.py")


def finished(log):
    if not os.path.exists(log):
        return set()
    return {r["cell"] for r in map(json.loads, open(log)) if r.get("stop_reason")}


def git(*args):
    return subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True).stdout.strip()


def write_run_txt(run_dir, args):
    lines = [
        f"started: {time.strftime('%Y-%m-%dT%H:%M:%S%z')}",
        f"commit: {git('rev-parse', 'HEAD')}",
        f"dirty: {bool(git('status', '--porcelain'))}",
        f"host_avx2: {'avx2' in open('/proc/cpuinfo').read()}",
        f"args: {json.dumps(vars(args))}",
    ]
    with open(os.path.join(run_dir, "RUN.txt"), "a") as f:
        f.write("\n".join(lines) + "\n\n")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--run-id", default=time.strftime("desc_%Y%m%d_%H%M%S"))
    p.add_argument("--models", nargs="+", required=True)
    p.add_argument("--kernels", nargs="+", required=True)
    p.add_argument("--levels", nargs="+", default=["L0", "L1", "L2", "L3"])
    p.add_argument("--repeats", type=int, default=1)
    p.add_argument("--max-repairs", type=int, default=0)
    p.add_argument("--feedback", default="none")
    args = p.parse_args()

    run_dir = os.path.join(ROOT, "results", args.run_id)
    os.makedirs(run_dir, exist_ok=True)
    write_run_txt(run_dir, args)
    done = finished(os.path.join(run_dir, "calls.jsonl"))

    for repeat in range(args.repeats):
        for kernel in args.kernels:
            for level in args.levels:
                for model in args.models:
                    if cell_id(kernel, level, model, repeat) in done:
                        continue
                    subprocess.run([sys.executable, GEN, "--kernel", kernel, "--level", level,
                                    "--model", model, "--run-dir", run_dir, "--repeat", str(repeat),
                                    "--max-repairs", str(args.max_repairs), "--feedback", args.feedback])

    print(f"\nfunnel: python3 scripts/desc_funnel.py {run_dir}")


if __name__ == "__main__":
    main()
