#!/usr/bin/env python3
# one cell: description -> model -> .S -> build -> run driver, one log record per round
# usage: python3 scripts/gen_desc.py --kernel atax --level L2 --model Qwen/Qwen3.8-27B --run-dir results/desc_test
#        --reply-file reply.txt uses a saved reply instead of calling the API

import argparse
import hashlib
import json
import os
import glob
import re
import signal
import subprocess
import sys
import time

import llm_client
from llm_client import LlmError, append_call
from llm_runner import run_binary

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DESC = os.path.join(ROOT, "kernels", "desc")
PROMPT = os.path.join(ROOT, "prompts", "desc_x86_64.txt")
REPAIR = os.path.join(ROOT, "prompts", "desc_repair.txt")

THINK_RE = re.compile(r"<\s*(?:\w+:)?think(?:ing)?\s*>.*?<\s*/\s*(?:\w+:)?think(?:ing)?\s*>", re.S | re.I)
FENCE_RE = re.compile(r"```[^\n]*\n(.*?)```", re.S)
SIZE_RE = re.compile(r"^size (\S+): (pass|fail)", re.M)
ERR_RE = re.compile(r"Error:|error:|undefined reference")
RET_RE = re.compile(r"^\s*(?:[\w.]+:\s*)?(?:rep\s+|repz\s+)?ret[qlw]?\b", re.M)
TIMEOUT_SIGNALS = {signal.SIGXCPU, signal.SIGKILL}


def model_label(model, reasoning_effort=None):
    return f"{model}@reasoning-{reasoning_effort}" if reasoning_effort else model


def cell_id(kernel, level, model, repeat):
    return f"{kernel}__{level}__{model.replace('/', '_')}__rep{repeat}"


def log_files(run_dir):
    files = glob.glob(os.path.join(run_dir, "calls.jsonl")) + glob.glob(os.path.join(run_dir, "calls", "*.jsonl"))
    return sorted(files)


def load_records(run_dir):
    records = []
    for path in log_files(run_dir):
        records += [json.loads(line) for line in open(path) if line.strip()]
    return records


def sha(text):
    return hashlib.sha256(text.encode()).hexdigest()[:12]


def fill(template_path, values):
    text = open(template_path).read()
    for key, value in values.items():
        text = text.replace("{{" + key + "}}", value)
    return text


def load_kernel(kernel, level):
    kdir = os.path.join(DESC, kernel)
    spec = json.load(open(os.path.join(kdir, "spec.json")))
    description = open(os.path.join(kdir, f"{level}.txt")).read().strip()
    return kdir, spec, description


def build_prompt(spec, description):
    return fill(PROMPT, {"NAME": spec["name"], "SIGNATURE": spec["signature"],
                         "LAYOUT": spec["layout"], "DESCRIPTION": description})


def extract_asm(reply, name):
    text = THINK_RE.sub("", reply)
    globl = re.compile(r"\.globa?l\s+" + re.escape(name) + r"\b")
    blocks = [b for b in FENCE_RE.findall(text) if globl.search(b)]
    if blocks:
        return blocks[-1], "fenced" if len(blocks) == 1 else f"last_of_{len(blocks)}_blocks"
    if globl.search(text):
        return text, "unfenced"
    return None, "no_globl"


def error_lines(stderr, limit=20):
    seen = []
    for line in stderr.splitlines():
        line = re.sub(r"^[^\s:]+:(\d+):", r"line \1:", line)
        line = re.sub(r"^\S+\.o:\s*", "", line)
        if ERR_RE.search(line) and line not in seen:
            seen.append(line)
    return seen[:limit]


def build(asm_path, kdir, exe):
    obj = exe + ".o"
    r = subprocess.run(["gcc", "-c", asm_path, "-o", obj], capture_output=True, text=True)
    if r.returncode:
        return "assemble_fail", r.stderr
    r = subprocess.run(["gcc", "-O2", "-I", os.path.join(DESC, "common"),
                        os.path.join(kdir, "driver.c"), os.path.join(kdir, "ref.c"),
                        obj, "-o", exe, "-lm"], capture_output=True, text=True)
    if r.returncode:
        return "link_fail", r.stderr
    return None, ""


def run(exe):
    out = exe + ".out"
    rc, timed_out = run_binary(exe, out, exe + ".err")
    text = open(out, errors="replace").read() if os.path.exists(out) else ""
    sizes = dict(SIZE_RE.findall(text))
    passed = ", ".join(s for s, v in sizes.items() if v == "pass") or "none"
    info = {"exit_code": rc, "signal": None}
    if rc < 0:
        info["signal"] = signal.Signals(-rc).name
    if timed_out or (rc < 0 and -rc in TIMEOUT_SIGNALS):
        return "timeout", sizes, text + f"\nThe program did not finish in time (sizes passed before: {passed}).", info
    if rc < 0:
        return "crash", sizes, text + f"\nThe program crashed with {info['signal']} (sizes passed before the crash: {passed}).", info
    return ("pass" if rc == 0 and sizes else "wrong"), sizes, text, info


def check(asm_path, kdir, exe):
    if not RET_RE.search(open(asm_path).read()):
        return "incomplete", {}, "The function contains no ret instruction; the answer looks unfinished.", {}
    stage, log = build(asm_path, kdir, exe)
    if stage:
        return stage, {}, log, {}
    return run(exe)


def fail_lines(details, limit=5):
    return [l for l in details.splitlines() if ": fail " in l][:limit]


def feedback(stage, details, mode):
    if mode == "none":
        return None
    if stage in ("extract_fail", "incomplete"):
        return details
    if stage in ("assemble_fail", "link_fail"):
        return "Your code did not build:\n" + "\n".join(error_lines(details))
    if mode == "compile+tests" and stage in ("wrong", "crash", "timeout"):
        return f"Your code built but failed the tests ({stage}). Test output:\n{details[:4000]}"
    return None


def ask(args, prompt):
    if args.reply_file:
        return {"content": open(args.reply_file).read(), "underlying_model": "offline",
                "finish_reason": "offline", "sampling_params": {}, "usage": {},
                "cost_usd": 0.0, "elapsed_seconds": 0.0, "reasoning": None}
    prompt += f"\n/* call-id: {llm_client.new_call_id()} */\n"
    return llm_client.call_llm(args.model, prompt, temperature=args.temperature, seed=args.seed,
                               reasoning_effort=args.reasoning_effort)


def save(path, text):
    with open(path, "w") as f:
        f.write(text)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--kernel", required=True)
    p.add_argument("--level", required=True)
    p.add_argument("--model", required=True)
    p.add_argument("--run-dir", required=True)
    p.add_argument("--repeat", type=int, default=0)
    p.add_argument("--max-repairs", type=int, default=0)
    p.add_argument("--feedback", choices=["none", "compile", "compile+tests"], default="none")
    p.add_argument("--temperature", type=float)
    p.add_argument("--seed", type=int)
    p.add_argument("--reasoning-effort", help="e.g. none: passed to the gateway, cells get a separate label")
    p.add_argument("--reply-file")
    p.add_argument("--log", help="log file (default: <run-dir>/calls.jsonl)")
    args = p.parse_args()
    if not args.reply_file and not llm_client.API_KEY:
        sys.exit("set RIVAULT_API_KEY first")

    kdir, spec, description = load_kernel(args.kernel, args.level)
    label = model_label(args.model, args.reasoning_effort)
    cell = cell_id(args.kernel, args.level, label, args.repeat)
    code_dir = os.path.join(args.run_dir, "code")
    raw_dir = os.path.join(args.run_dir, "raw")
    os.makedirs(code_dir, exist_ok=True)
    os.makedirs(raw_dir, exist_ok=True)
    log = args.log or os.path.join(args.run_dir, "calls.jsonl")

    base = build_prompt(spec, description)
    prompt, prev_sig = base, None
    for rnd in range(args.max_repairs + 1):
        tag = f"{cell}.r{rnd}"
        rec = {"time": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
               "run": os.path.basename(os.path.normpath(args.run_dir)), "cell": cell,
               "kernel": args.kernel, "level": args.level, "model": label,
               "api_model": args.model, "reasoning_effort": args.reasoning_effort,
               "repeat": args.repeat, "round": rnd, "feedback": args.feedback if rnd else None,
               "prompt_sha": sha(prompt)}
        try:
            r = ask(args, prompt)
        except LlmError as e:
            rec.update(stage="no_answer", error=str(e), usage=e.usage, cost_usd=e.cost_usd,
                       elapsed_seconds=e.elapsed_seconds, attempts=e.attempts,
                       stop_reason="api_error")
            append_call(log, rec)
            print(f"{tag}: no_answer ({e})")
            return

        rec.update(underlying_model=r["underlying_model"], finish_reason=r["finish_reason"],
                   sampling_params=r["sampling_params"], usage=r["usage"],
                   cost_usd=r["cost_usd"], elapsed_seconds=r["elapsed_seconds"],
                   attempts=r.get("attempts"))
        save(os.path.join(raw_dir, tag + ".txt"), r["content"])
        if r.get("reasoning"):
            save(os.path.join(raw_dir, tag + ".reasoning.txt"), r["reasoning"])

        asm, how = extract_asm(r["content"], spec["name"])
        rec["extract_reason"] = how
        if asm is None:
            stage, sizes, details, info = "extract_fail", {}, f"Your answer contained no .globl {spec['name']}.", {}
        else:
            asm_path = os.path.join(code_dir, tag + ".S")
            save(asm_path, asm.rstrip() + "\n")
            rec["asm_sha"] = sha(asm)
            stage, sizes, details, info = check(asm_path, kdir, os.path.join(code_dir, tag))

        errors = error_lines(details) if stage in ("assemble_fail", "link_fail") else []
        failures = fail_lines(details) if stage in ("wrong", "crash", "timeout") else []
        shape = [re.sub(r"[-\d.e+]+", "#", l) for l in failures]
        sig = sha(stage + "\n".join(sorted(errors)) + "\n".join(shape) + json.dumps(sizes, sort_keys=True))
        rec.update(stage=stage, sizes=sizes, sizes_passed=sum(v == "pass" for v in sizes.values()),
                   first_error=errors[0] if errors else None, error_lines=errors[:5],
                   test_failures=failures, signal=info.get("signal"), exit_code=info.get("exit_code"),
                   error_sig=sig)

        note = feedback(stage, details, args.feedback)
        if stage == "pass":
            stop = "pass"
        elif rnd == args.max_repairs:
            stop = "budget"
        elif note is None:
            stop = "no_feedback"
        elif sig == prev_sig:
            stop = "same_errors"
        else:
            stop = None
        rec["stop_reason"] = stop
        append_call(log, rec)
        print(f"{tag}: {stage}" + (f" ({stop})" if stop else ""))
        if stop:
            return

        prev_sig = sig
        prompt = base + "\n\n" + fill(REPAIR, {"ASM": asm or r["content"][:8000], "FEEDBACK": note})


if __name__ == "__main__":
    main()
