#!/usr/bin/env python3
# generate -> compile -> (feed errors back, model fixes) xN -> compile, in one place.
# The bash run scripts delegate here and keep the run/diff orchestration.

import argparse
import json
import os
import re
import shutil
import subprocess
import sys

import llm_client
from llm_client import (API_KEY, LlmError, append_call, build_prompt, clean_region,
                        new_call_id, save_reasoning, split_scop, splice)

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_REPAIR_TEMPLATE = os.path.join(HERE, "..", "prompts", "repair_template.txt")
DEFAULT_PROMPT = os.path.join(HERE, "..", "prompts", "i8086_prompt_template.txt")

# messages that mean "compiler rejected our asm" (mirrors funnel.py / analyze_errors.py)
ERR_TAGS = ("Error:", " error:", "undefined reference")

# GCC context/echo lines that add no signal (caret lines, source echo, macro defs)
_CONTEXT_EQ = ("^", "|", "# define", "   int cs", "   const char")


def slug(alias):
    return alias.replace("/", "_")


# --- prompt composition ---------------------------------------------------------

def build_repair_prompt(base_template_path, kernel_source, history, repair_template_path,
                        call_id):
    """Compose the repair message: the same base prompt (full kernel + target rules) plus
    a tail with the last failed region and ALL prior deduped errors (so a fix already
    applied isn't reintroduced). Uses str.replace, not .format(), because the previous
    asm contains literal braces."""
    base = build_prompt(base_template_path, kernel_source, None)  # nonce lives in the tail
    tail = open(repair_template_path).read()
    tail = tail.replace("{{PREVIOUS_CODE}}", _cap(history[-1]["code"], 8192))
    errs = "\n----\n".join(h["errors"] for h in history)
    tail = tail.replace("{{COMPILER_ERRORS}}", _cap(errs, 4096))
    return base + "\n\n" + tail + f"\n/* call-id: {call_id} */\n"


def _cap(text, limit):
    if len(text) <= limit:
        return text
    return text[:limit] + "\n... [truncated]"


# --- compiler-error extraction ------------------------------------------------

def extract_errors(log_path, max_chars=4096):
    """Deduplicated, order-preserving compiler errors from a compile log.

    Drops polybench.c noise (the utility file floods every 8086 log with cache-size
    warnings), warning:/note: lines, collect2 header and caret/context echo lines. Keeps
    only lines carrying an error marker, deduped by message with file/line/col normalized.
    Mirrors funnel.py's patterns so pre-processing and classification agree."""
    if not os.path.exists(log_path):
        return ""
    seen, out = set(), []
    for line in open(log_path, errors="replace"):
        s = line.rstrip("\n")
        if "polybench.c" in s:
            continue
        if "warning:" in s or " note:" in s:
            continue
        if s.startswith("collect2"):
            continue
        if s.lstrip().startswith(_CONTEXT_EQ):
            continue
        if not any(t in s for t in ERR_TAGS):
            continue
        key = _norm_loc(s)
        if key in seen:
            continue
        seen.add(key)
        out.append(s)
        if sum(len(x) + 1 for x in out) >= max_chars:
            break
    return "\n".join(out)


def _norm_loc(s):
    # "<file>.c:<line>: Error: ..." and "<file>.c:<line>:<col>: error: ..." both ->
    # "<file>.c:N:<rest>" so the same message at many locations dedupes to one line.
    return re.sub(r"\.c:\d+(?::\d+)?(?=:)", ".c:N", s)


def run_compile(cc, flags, src, polybench_c, kernel_dir, out_exe, log_path, link_math,
                timeout=900):
    cmd = [cc] + flags.split() + ["-I", kernel_dir, polybench_c, src, "-o", out_exe]
    if link_math:
        cmd.append("-lm")
    try:
        with open(log_path, "w") as f:
            proc = subprocess.run(cmd, stderr=subprocess.STDOUT, stdout=f, timeout=timeout)
        return proc.returncode
    except subprocess.TimeoutExpired:
        with open(log_path, "a") as f:
            f.write(f"\n[compile timed out after {timeout}s]\n")
        return 1


# --- record/log helpers ---------------------------------------------------------

def _max_tokens_for(args, attempt):
    # attempt 0 uses args.max_tokens (may be None = no cap); repair turns use the
    # fixed repair_max_tokens cap. Record what was actually sent to the API so the
    # logs distinguish "hit the repair cap" from "was uncapped" (see 504/empty-content analysis).
    return args.max_tokens if attempt == 0 else args.repair_max_tokens


def base_record(args, attempt, model, elapsed, reasoning_file):
    return {"kernel": args.kernel,
            "alias_requested": args.alias,
            "underlying_model": model,
            "dry_run": args.dry_run,
            "elapsed_seconds": round(elapsed, 1),
            "reasoning_file": reasoning_file,
            "max_tokens_requested": _max_tokens_for(args, attempt),
            "usage": {},
            "cost_usd": None,
            "key_spend_total_usd": None,
            "prompt_template": args.prompt_template,
            "repair_attempt": attempt,
            "attempt_kind": "generate" if attempt == 0 else "repair"}


def error_record(args, attempt, err):
    """JSONL line for a failed API call (no usable reply), mirrors generate_asm.py."""
    return {"kernel": args.kernel,
            "alias_requested": args.alias,
            "underlying_model": f"error: {err}",
            "dry_run": args.dry_run,
            "elapsed_seconds": round(err.elapsed_seconds or 0, 1),
            "reasoning_file": None,
            "max_tokens_requested": _max_tokens_for(args, attempt),
            "usage": err.usage or {},
            "cost_usd": err.cost_usd,
            "key_spend_total_usd": err.key_spend,
            "prompt_template": args.prompt_template,
            "repair_attempt": attempt,
            "attempt_kind": "generate" if attempt == 0 else "repair"}


def write_calls(log_path, call_records, compiled):
    """Append every call record; the last one carries final_compiled."""
    for i, rec in enumerate(call_records):
        if i == len(call_records) - 1:
            rec["final_compiled"] = compiled
        append_call(log_path, rec)


def write_status(status_path, fields):
    with open(status_path, "w") as f:
        json.dump(fields, f)


# --- the loop --------------------------------------------------------------------

def main():
    p = argparse.ArgumentParser(
        description="generate #pragma scop inline asm, compile, repair on failure")
    p.add_argument("--kernel", required=True)
    p.add_argument("--src", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--alias", required=True)
    p.add_argument("--log", default="results/llm_calls.jsonl")
    p.add_argument("--prompt-template", default=DEFAULT_PROMPT)
    p.add_argument("--repair-template", default=DEFAULT_REPAIR_TEMPLATE)
    p.add_argument("--work-dir", required=True)
    p.add_argument("--cc", choices=["gcc", "ia16-elf-gcc"], required=True)
    p.add_argument("--polybench-c", required=True)
    p.add_argument("--kernel-dir", required=True)
    p.add_argument("--extra-c-flags", default="-O0 -DMINI_DATASET -DPOLYBENCH_DUMP_ARRAYS")
    p.add_argument("--exe", required=True)
    p.add_argument("--link-math", action="store_true")
    p.add_argument("--max-repairs", type=int, default=int(os.environ.get("MAX_REPAIRS", 2)))
    p.add_argument("--repair-max-tokens", type=int, default=None,
                   help="cap for repair-turn output tokens (None = uncapped, like turn 0). "
                        "Justified by logs: 32K caps made reasoning-chain repair calls "
                        "return finish_reason=length with no content (wasted calls, api_error rows).")
    p.add_argument("--max-tokens", type=int, default=None, help="omit for no cap (initial call)")
    p.add_argument("--timeout", type=int, default=900)
    p.add_argument("--dry-run", action="store_true")
    p.add_argument("--dry-run-fail-iters", type=int, default=None,
                   help="(dry-run) fake that many compile failures before success")
    p.add_argument("--dry-run-error-file", default=None,
                   help="(dry-run) feed this real captured compile log as canned errors")
    p.add_argument("--no-stop-on-same-errors", action="store_true",
                   help="do not stop when a repair repeats the previous round's errors")
    args = p.parse_args()

    os.makedirs(args.work_dir, exist_ok=True)
    os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
    os.makedirs(os.path.dirname(args.log) or ".", exist_ok=True)

    slug_ = slug(args.alias)
    compile_log = os.path.join(args.work_dir, f"{args.kernel}__{slug_}.compile_err.log")
    status_path = os.path.join(args.work_dir, f"{args.kernel}__{slug_}.status")

    src = open(args.src).read()
    try:
        before, region, after = split_scop(src)
    except ValueError as e:
        sys.exit(str(e))

    cc = shutil.which("ia16-elf-gcc") or "ia16-elf-gcc" if args.cc == "ia16-elf-gcc" \
        else (shutil.which("gcc") or "gcc")

    history = []            # {"round", "code", "errors"} per failed round
    repairs = 0
    compiled = False
    last_model = "unknown"
    prev_errors = None
    call_records = []

    for attempt in range(0, args.max_repairs + 1):
        if attempt == 0:
            if args.dry_run:
                content = "  /* DRY RUN PLACEHOLDER */\n" + region
                model, elapsed, reasoning = "dry-run", 0, None
            else:
                call_id = new_call_id()
                prompt = build_prompt(args.prompt_template, src, call_id)
                print(f"[{args.kernel}] calling {args.alias} (attempt 0, "
                      f"max_tokens={args.max_tokens or 'none'}) ...")
                r = _call(args, prompt, attempt)
                model, elapsed, reasoning = (r["underlying_model"], r["elapsed_seconds"],
                                             r["reasoning"])
                last_model = model
                content = r["content"]
        else:
            if args.dry_run:
                content = (history[-1]["code"] if history else region)  # canned reply
                model, elapsed, reasoning = "dry-run", 0, None
            else:
                call_id = new_call_id()
                prompt = build_repair_prompt(args.prompt_template, src, history,
                                             args.repair_template, call_id)
                print(f"[{args.kernel}] calling {args.alias} (repair attempt {attempt}) ...")
                r = _call(args, prompt, attempt)
                model, elapsed, reasoning = (r["underlying_model"], r["elapsed_seconds"],
                                             r["reasoning"])
                last_model = model
                content = r["content"]

        reasoning_file = save_reasoning(args.log, args.kernel, slug_,
                                        f"repair{attempt}" if attempt else "", reasoning)

        rec = base_record(args, attempt, model, elapsed, reasoning_file)
        if not args.dry_run:
            # real API call: carry the actual usage/cost so evaluate.py totals include it
            rec.update({"usage": r.get("usage", {}),
                        "cost_usd": r.get("cost_usd"),
                        "key_spend_total_usd": r.get("key_spend_total_usd")})

        replacement = clean_region(content) if not args.dry_run else content.strip()
        if replacement is None:
            # nothing salvageable -> consume the attempt, don't feed garbage to the compiler
            print(f"[{args.kernel}] attempt {attempt}: no usable asm block extracted, "
                  "skipping compile")
            rec["repair_extracted"] = False
            call_records.append(rec)
            if attempt == 0:
                break                       # nothing usable at all -> status compiled=false
            repairs = attempt
            continue

        rec["repair_extracted"] = True
        rec["previous_region_len"] = len(history[-1]["code"]) if history else 0
        call_records.append(rec)

        gen_src = splice(before, replacement, after)
        with open(args.out, "w") as f:
            f.write(gen_src)
        snapshot = os.path.join(args.work_dir, f"{args.kernel}__{slug_}.attempt{attempt}.c")
        with open(snapshot, "w") as f:
            f.write(gen_src)

        # compile (or fake it in dry-run)
        canned = ""
        if not args.dry_run:
            rc = run_compile(cc, args.extra_c_flags, snapshot, args.polybench_c,
                             args.kernel_dir, args.exe, compile_log,
                             args.link_math, timeout=args.timeout)
        elif args.dry_run_fail_iters is not None and attempt < args.dry_run_fail_iters:
            rc = 1
            if args.dry_run_error_file and os.path.exists(args.dry_run_error_file):
                canned = extract_errors(args.dry_run_error_file, 4096) or \
                    "Error: operand size mismatch for `addsd'"
            else:
                canned = "Error: operand size mismatch for `addsd'"
            with open(compile_log, "w") as f:
                f.write(canned + "\n")
        else:
            rc = 0

        if rc == 0:
            compiled = True
            print(f"[{args.kernel}] attempt {attempt} COMPILED (repairs so far: {repairs})")
            break

        errs = canned or extract_errors(compile_log, 4096)
        if not errs:
            print(f"[{args.kernel}] attempt {attempt}: compile failed with no extractable "
                  "errors, stopping")
            break
        if (not args.no_stop_on_same_errors and prev_errors is not None
                and errs == prev_errors):
            print(f"[{args.kernel}] attempt {attempt}: same errors as previous round, "
                  "stopping (no progress)")
            break
        prev_errors = errs
        history.append({"round": attempt, "code": replacement, "errors": errs})
        if attempt > 0:
            repairs = attempt      # a repair turn was actually issued
        print(f"[{args.kernel}] attempt {attempt} failed, feeding errors back "
              f"(repairs issued: {repairs})")
        if attempt == args.max_repairs:
            print(f"[{args.kernel}] repair budget exhausted ({args.max_repairs})")
            break

    write_calls(args.log, call_records, compiled)
    write_status(status_path, {"kernel": args.kernel,
                               "alias_requested": args.alias,
                               "underlying_model": last_model,
                               "compiled": compiled,
                               "repairs": repairs,
                               "compile_log": compile_log})
    print(f"[{args.kernel}] finished: compiled={compiled} repairs={repairs}")


def _call(args, prompt, attempt):
    """One real LLM call. On failure, log the error entry and exit so bash records api_error."""
    try:
        return llm_client.call_llm(args.alias, prompt,
                                   max_tokens=(args.max_tokens if attempt == 0
                                               else args.repair_max_tokens),
                                   timeout=args.timeout)
    except LlmError as e:
        print(f"[{args.kernel}] API call failed (attempt {attempt}): {e}")
        append_call(args.log, error_record(args, attempt, e))
        sys.exit(1)


if __name__ == "__main__":
    main()
