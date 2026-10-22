#!/usr/bin/env python3
# run a matrix of description->asm cells, skipping cells that already finished
# usage: python3 scripts/run_desc.py --run-id desc_pilot --models Qwen/Qwen3.8-27B zai-org/GLM-5.3-Flash \
#            --kernels axpy atax --levels L0 L2 L3 --repeats 2 [--shard name]
# with --shard, this process logs to <run>/calls/<name>.jsonl, so several processes can share one run
# --target c: the model writes C instead of assembly (baseline)

import argparse
import itertools
import json
import os
import subprocess
import sys
import time

from desc_repair import trajectories
from gen_desc import C_FLAGS, cell_id, load_records, model_label
from llm_client import API_KEY

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GEN = os.path.join(ROOT, "scripts", "gen_desc.py")


def finished(run_dir):
    done = set()
    for r in load_records(run_dir):
        if r.get("stop_reason") == "api_error" and r.get("round", 0) == 0:
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
        f"dirty: {bool(git('status', '--porcelain', '--untracked-files=no'))}",
        f"host_avx2: {'avx2' in open('/proc/cpuinfo').read()}",
        f"nonce: {not args.no_nonce}",
        f"feedback: {args.feedback}",
        f"start_from: {args.start_from}",
        f"target: {args.target}" + (f" (gcc {' '.join(C_FLAGS)})" if args.target == "c" else ""),
        f"base_model: {args.base_model or ', '.join(args.models)}",
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
    p.add_argument("--target", choices=["asm", "c"], default="asm", help="c: the model writes C, compiled with gcc C_FLAGS")
    p.add_argument("--base-model", help="the model behind an alias such as RiVault/Reasoning-Medium, logged as base_model")
    p.add_argument("--max-errors", type=int, default=5, help="API errors in a row before pausing")
    p.add_argument("--pause", type=int, default=900, help="seconds to wait after --max-errors")
    p.add_argument("--max-pauses", type=int, default=3, help="pauses in a row before stopping")
    p.add_argument("--shard", help="name of this process; logs to <run>/calls/<shard>.jsonl")
    p.add_argument("--temperature", type=float)
    p.add_argument("--seed", type=int, help="same seed for every call")
    p.add_argument("--seed-per-repeat", action="store_true", help="use seed + repeat instead of the same seed")
    p.add_argument("--no-nonce", action="store_true", help="no call-id in prompts and no gateway cache: identical cells send identical requests")
    p.add_argument("--start-from", help="run dir: only cells that failed round 0 there, starting from that reply")
    args = p.parse_args()
    if not API_KEY:
        sys.exit("set RIVAULT_API_KEY first")
    if args.start_from:
        args.start_from = os.path.abspath(args.start_from)

    run_dir = os.path.join(ROOT, "results", args.run_id)
    mode = "c" if args.target == "c" else "direct"
    mixed = ({(r.get("nonce", True), r.get("start_from"), r.get("mode", "direct")) for r in load_records(run_dir)}
             - {(not args.no_nonce, args.start_from, mode)})
    if mixed:
        sys.exit(f"{args.run_id} already has cells with other settings (nonce, start_from, mode) {mixed}; use a new --run-id")
    todo = None
    if args.start_from:
        todo = {cell for cell, rounds in trajectories(load_records(args.start_from)).items()
                if rounds[0]["stage"] not in ("pass", "no_answer")}
    log = os.path.join(run_dir, "calls", f"{args.shard}.jsonl") if args.shard else os.path.join(run_dir, "calls.jsonl")
    os.makedirs(os.path.dirname(log), exist_ok=True)
    write_run_txt(run_dir, args)
    done = finished(run_dir)

    errors = pauses = 0
    cells = itertools.product(range(args.repeats), args.kernels, args.levels, args.models)
    for repeat, kernel, level, model in cells:
        cell = cell_id(kernel, level, model_label(model, args.reasoning_effort), repeat)
        if cell in done or (todo is not None and cell not in todo):
            continue
        subprocess.run([sys.executable, GEN, "--kernel", kernel, "--level", level,
                        "--model", model, "--run-dir", run_dir, "--repeat", str(repeat),
                        "--max-repairs", str(args.max_repairs), "--feedback", args.feedback, "--log", log]
                       + (["--no-nonce"] if args.no_nonce else [])
                       + (["--target", args.target] if args.target != "asm" else [])
                       + (["--base-model", args.base_model] if args.base_model else [])
                       + (["--start-from", args.start_from] if args.start_from else [])
                       + (["--reasoning-effort", args.reasoning_effort] if args.reasoning_effort else [])
                       + (["--temperature", str(args.temperature)] if args.temperature is not None else [])
                       + (["--seed", str(args.seed + (repeat if args.seed_per_repeat else 0))] if args.seed is not None else []))

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
