#!/usr/bin/env python3
# one cell: description -> model -> .S -> build -> run driver, one log record per round
# usage: python3 scripts/gen_desc.py --kernel atax --level L2 --model Qwen/Qwen3.8-27B --run-dir results/desc_test
#        --reply-file reply.txt uses a saved reply instead of calling the API
#        --start-from results/desc_full takes round 0 from that run's saved reply for the same cell

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
FAULT_RE = re.compile(r"^fault signal=(\d+) size=(\S+) ip_from_tramp=(0x[0-9a-f]+) addr=(0x[0-9a-f]+)\n?", re.M)
LINE_RE = re.compile(r"^line (\d+)\b")
V2 = "compile+tests+v2"
STAGE_WORDS = {"extract_fail": "no function found", "incomplete": "no ret instruction",
               "assemble_fail": "did not assemble", "link_fail": "did not link", "crash": "crashed",
               "timeout": "did not finish in time", "wrong": "wrong results"}


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


def build(asm_path, kdir, exe, diag=False, debug=False):
    obj = exe + ".o"
    g = ["-g"] if debug else []
    r = subprocess.run(["gcc", *g, "-c", asm_path, "-o", obj], capture_output=True, text=True)
    if r.returncode:
        return "assemble_fail", r.stderr
    flags = g + (["-DDESC_DIAG", "-D_GNU_SOURCE"] if diag else [])
    r = subprocess.run(["gcc", "-O2", *flags, "-I", os.path.join(DESC, "common"),
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


def check(asm_path, kdir, exe, diag=False):
    if not RET_RE.search(open(asm_path).read()):
        return "incomplete", {}, "The function contains no ret instruction; the answer looks unfinished.", {}
    stage, log = build(asm_path, kdir, exe, diag)
    if stage:
        return stage, {}, log, {}
    return run(exe)


def text_symbols(exe):
    out = subprocess.run(["nm", "--defined-only", exe], capture_output=True, text=True).stdout
    return {name: int(addr, 16) for addr, kind, name in (l.split() for l in out.splitlines() if len(l.split()) == 3)
            if kind in "Tt"}


def tool(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True).stdout


def quote(lines, n):
    return f"    {n} | {lines[n - 1].strip()}" if 0 < n <= len(lines) else None


def crash_site(details, asm_path, kdir, exe):
    m = FAULT_RE.search(details)
    if not m:
        if "SIGABRT" not in details:
            return None, ""
        err = open(exe + ".err", errors="replace").read().strip().splitlines() if os.path.exists(exe + ".err") else []
        return "abort", ("The C library aborted the test program" + (f" with \"{err[-1]}\"" if err else "") +
                         ". This usually means your code wrote outside the arrays it was given.")
    signo, size, offset, addr = int(m.group(1)), m.group(2), int(m.group(3), 16), int(m.group(4), 16)
    name = signal.Signals(signo).name
    symbols = text_symbols(exe)
    ip = (symbols["abi_tramp"] + offset) % 2**64
    head = f"The {name} happened at size {size}"
    tail = f"\nThe faulting memory address was {addr:#x}." if name in ("SIGSEGV", "SIGBUS") else ""
    insn = tool("objdump", "-d", "--no-show-raw-insn", f"--start-address={ip}", f"--stop-address={ip + 16}", exe)
    insn = next((l.split(":", 1)[1].strip() for l in insn.splitlines() if re.match(rf"\s*{ip:x}:", l)), None)
    if insn is None and ip == addr:
        return "invalid address", (f"{head}: your code jumped or returned to the invalid address {ip:#x} "
                                   "(for example a ret with an unbalanced stack).")
    if insn is None:
        return "library", (f"{head} inside a C library function called by the test program, not inside your "
                           "function. This usually means your code wrote outside the arrays it was given." + tail)
    where = None
    if not build(asm_path, kdir, exe + ".g", diag=True, debug=True)[0] and text_symbols(exe + ".g") == symbols:
        where = tool("addr2line", "-e", exe + ".g", f"{ip:#x}").strip()
    file, _, line = (where or "?:?").rpartition(":")
    file, line = os.path.basename(file), re.match(r"\d+", line)
    if file in ("driver.c", "ref.c", "check.h", "check_int.h"):
        return "outside your function", (f"{head} in the test program ({file}), not inside your function "
                                         f"(instruction: {insn}). This usually means your function changed memory "
                                         "or registers it should have left alone." + tail)
    if file != "??" and line:
        n = int(line.group())
        source = quote(open(asm_path).read().splitlines(), n)
        return f"line {n}", f"{head}, at line {n} of your code:\n{source}\n(instruction: {insn})" + tail
    return "unknown line", f"{head}, at instruction: {insn}" + tail


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


def history_line(rec):
    line = f"- round {rec['round']}: {STAGE_WORDS[rec['stage']]}"
    if rec["signal"] and rec["stage"] == "crash":
        line += f" with {rec['signal']}"
    if rec["stage"] in ("wrong", "crash", "timeout"):
        line += ", sizes passed: " + (", ".join(s for s, v in rec["sizes"].items() if v == "pass") or "none")
    detail = rec["first_error"] or rec.get("crash_location") or next(iter(rec["test_failures"]), None)
    return line + (f" ({detail})" if detail else "")


def feedback_v2(stage, details, asm, crash_text, history):
    if stage == "pass":
        return None
    if stage in ("extract_fail", "incomplete"):
        note = details
    elif stage in ("assemble_fail", "link_fail"):
        lines = (asm or "").splitlines()
        out = []
        for error in error_lines(details):
            m = LINE_RE.match(error)
            out += [error] + [q for q in [m and quote(lines, int(m.group(1)))] if q]
        note = "Your code did not build:\n" + "\n".join(out)
    else:
        output = re.sub(r"\n{2,}", "\n", FAULT_RE.sub("", details)).strip()
        note = f"Your code built but failed the tests ({stage}). Test output:\n{output[:4000]}"
        if crash_text:
            note += "\n" + crash_text
    if len(history) > 1:
        note = note.rstrip() + "\n\nSummary of all your answers so far (the last one is shown above):\n" + "\n".join(map(history_line, history))
    return note


def ask(args, prompt, reply_file=None):
    if reply_file:
        return {"content": open(reply_file).read(), "underlying_model": "offline",
                "finish_reason": "offline", "sampling_params": {}, "usage": {},
                "cost_usd": 0.0, "elapsed_seconds": 0.0, "reasoning": None}
    if not args.no_nonce:
        prompt += f"\n/* call-id: {llm_client.new_call_id()} */\n"
    return llm_client.call_llm(args.model, prompt, temperature=args.temperature, seed=args.seed,
                               reasoning_effort=args.reasoning_effort, no_cache=args.no_nonce)


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
    p.add_argument("--feedback", choices=["none", "compile", "compile+tests", V2], default="none")
    p.add_argument("--temperature", type=float)
    p.add_argument("--seed", type=int)
    p.add_argument("--reasoning-effort", help="e.g. none: passed to the gateway, cells get a separate label")
    p.add_argument("--no-nonce", action="store_true", help="no call-id in the prompt and no gateway cache")
    p.add_argument("--reply-file")
    p.add_argument("--start-from", help="run dir whose saved round-0 reply for this cell replaces the first call")
    p.add_argument("--log", help="log file (default: <run-dir>/calls.jsonl)")
    args = p.parse_args()
    if not args.reply_file and not llm_client.API_KEY:
        sys.exit("set RIVAULT_API_KEY first")

    kdir, spec, description = load_kernel(args.kernel, args.level)
    label = model_label(args.model, args.reasoning_effort)
    cell = cell_id(args.kernel, args.level, label, args.repeat)
    first_reply = args.reply_file
    if args.start_from:
        first_reply = os.path.join(args.start_from, "raw", cell + ".r0.txt")
        if not os.path.exists(first_reply):
            sys.exit(f"no round-0 reply {first_reply}")
    diag = args.feedback == V2
    history = []
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
               "feedback_mode": args.feedback, "nonce": not args.no_nonce, "start_from": args.start_from,
               "prompt_sha": sha(prompt)}
        try:
            r = ask(args, prompt, first_reply if rnd == 0 else args.reply_file)
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
                   attempts=r.get("attempts"), cache_headers=r.get("cache_headers"))
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
            stage, sizes, details, info = check(asm_path, kdir, os.path.join(code_dir, tag), diag)
        crash_where, crash_text = (crash_site(details, asm_path, kdir, os.path.join(code_dir, tag))
                                   if diag and stage == "crash" else (None, ""))

        errors = error_lines(details) if stage in ("assemble_fail", "link_fail") else []
        failures = fail_lines(details) if stage in ("wrong", "crash", "timeout") else []
        shape = [re.sub(r"[-\d.e+]+", "#", l) for l in failures]
        sig = sha(stage + "\n".join(sorted(errors)) + "\n".join(shape) + json.dumps(sizes, sort_keys=True))
        rec.update(stage=stage, sizes=sizes, sizes_passed=sum(v == "pass" for v in sizes.values()),
                   first_error=errors[0] if errors else None, error_lines=errors[:5],
                   test_failures=failures, signal=info.get("signal"), exit_code=info.get("exit_code"),
                   error_sig=sig, crash_location=crash_where)
        history.append(rec)

        if diag:
            note = feedback_v2(stage, details, asm, crash_text, history)
        else:
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
