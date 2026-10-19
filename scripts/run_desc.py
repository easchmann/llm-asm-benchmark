#!/usr/bin/env python3
# run a matrix of description->asm cells, skipping cells that already finished
# usage: python3 scripts/run_desc.py --run-id desc_pilot --models Qwen/Qwen3.8-27B zai-org/GLM-5.3-Flash \
#            --kernels axpy atax --levels L0 L2 L3 --repeats 2

import argparse
import itertools
import json
import os
import subprocess
import sys
import time

from gen_desc import cell_id, model_label
from llm_client import API_KEY

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GEN = os.path.join(ROOT, "scripts", "gen_desc.py")


def finished(log):
    if not os.path.exists(log):
        return set()
    done = set()
    for r in map(json.loads, open(log)):
        if r.get("stop_reason") == "api_error":
            done.discard(r["cell"])
        elif r.get("stop_reason"):
            done.add(r["cell"])
    return done


def last_record(log):
    if not os.path.exists(log):
        return None
    lines = open(log).read().splitlines()
    return json.loads(lines[-1]) if lines else None


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
    p.add_argument("--reasoning-effort", help="e.g. none: passed to the gateway, cells get a separate label")
    p.add_argument("--max-errors", type=int, default=5, help="API errors in a row before pausing")
    p.add_argument("--pause", type=int, default=900, help="seconds to wait after --max-errors")
    p.add_argument("--max-pauses", type=int, default=3, help="pauses in a row before stopping")
    args = p.parse_args()
    if not API_KEY:
        sys.exit("set RIVAULT_API_KEY first")

    run_dir = os.path.join(ROOT, "results", args.run_id)
    log = os.path.join(run_dir, "calls.jsonl")
    os.makedirs(run_dir, exist_ok=True)
    write_run_txt(run_dir, args)
    done = finished(log)

    errors = pauses = 0
    cells = itertools.product(range(args.repeats), args.kernels, args.levels, args.models)
    for repeat, kernel, level, model in cells:
        cell = cell_id(kernel, level, model_label(model, args.reasoning_effort), repeat)
        if cell in done:
            continue
        subprocess.run([sys.executable, GEN, "--kernel", kernel, "--level", level,
                        "--model", model, "--run-dir", run_dir, "--repeat", str(repeat),
                        "--max-repairs", str(args.max_repairs), "--feedback", args.feedback]
                       + (["--reasoning-effort", args.reasoning_effort] if args.reasoning_effort else []))

        last = last_record(log)
        if last and last["cell"] == cell and last["stage"] == "no_answer":
            errors += 1
        else:
            errors = pauses = 0
        if errors >= args.max_errors:
            if pauses >= args.max_pauses:
                sys.exit(f"{errors} API errors in a row after {pauses} pauses, stopping. "
                         "Run the same command again to resume.")
            pauses += 1
            print(f"{errors} API errors in a row, pausing {args.pause}s ({pauses}/{args.max_pauses})")
            time.sleep(args.pause)
            errors = 0

    print(f"\nfunnel: python3 scripts/desc_funnel.py {run_dir}")


if __name__ == "__main__":
    main()
