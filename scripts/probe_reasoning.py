#!/usr/bin/env python3
# which reasoning settings the gateway honours: a few short requests per model and setting, printing
# reasoning tokens, completion tokens, whether reasoning came back, time and the cost from the gateway headers
# usage: python3 scripts/probe_reasoning.py [--models Qwen/Qwen3.8-27B ...] [--settings default low] [--repeats 2]
#        --task gemm L3 sends that cell's real prompt instead of the arithmetic question (does a setting fit the 600 s limit?)
#        --glm-off tries GLM's own switches for turning reasoning off (template kwargs, thinking field, /nothink)
# every request is also appended to results/probe_reasoning.jsonl

import argparse
import os
import sys
import time

import llm_client
from gen_desc import ROOT, THINK_RE, build_prompt, load_kernel
from llm_client import LlmError, append_call
from md import heading, table

QUESTION = "What is 17 × 23? Answer with the number only."
PROBES = [
    ("zai-org/GLM-5.3-Flash", ["default", "none", "low"]),
    ("deepseek-ai/DeepSeek-V4-Flash", ["default", "low", "medium", "high"]),
    ("RiVault/Reasoning-Medium", ["default"]),
    ("Qwen/Qwen3.8-27B", ["none", "low", "default"]),
    ("moonshotai/Kimi-K3", ["none"]),
]
NO_THINK = {"enable_thinking": False}
GLM_OFF = [
    ("default", None, {}, ""),
    ("chat_template_kwargs", None, {"chat_template_kwargs": NO_THINK}, ""),
    ("chat_template_kwargs, allowed", None,
     {"chat_template_kwargs": NO_THINK, "allowed_openai_params": ["chat_template_kwargs"]}, ""),
    ("extra_body", None, {"extra_body": {"chat_template_kwargs": NO_THINK}}, ""),
    ("thinking disabled", None, {"thinking": {"type": "disabled"}}, ""),
    ("/nothink", None, {}, " /nothink"),
    ("effort minimal", "minimal", {}, ""),
]
MAX_TRIES = {"moonshotai/Kimi-K3": 1}


def reasoning_tokens(usage):
    return (usage.get("completion_tokens_details") or {}).get("reasoning_tokens")


def effort_setting(setting):
    return (setting, None if setting == "default" else setting, {}, "")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--models", nargs="+", help="default: all models in PROBES")
    p.add_argument("--settings", nargs="+", help="reasoning_effort values to send, 'default' sends none (default: per model)")
    p.add_argument("--glm-off", action="store_true", help="GLM only, with the settings in GLM_OFF")
    p.add_argument("--repeats", type=int, default=2)
    p.add_argument("--task", nargs=2, metavar=("KERNEL", "LEVEL"))
    p.add_argument("--log", default=os.path.join(ROOT, "results", "probe_reasoning.jsonl"))
    args = p.parse_args()
    if not llm_client.API_KEY:
        sys.exit("set RIVAULT_API_KEY first")
    llm_client.MAX_RETRIES = 0

    prompt = QUESTION
    if args.task:
        _, spec, description = load_kernel(*args.task)
        prompt = build_prompt(spec, description)
    if args.glm_off:
        probes = [("zai-org/GLM-5.3-Flash", [s for s in GLM_OFF if not args.settings or s[0] in args.settings])]
    else:
        probes = [(m, list(map(effort_setting, args.settings or s))) for m, s in PROBES if not args.models or m in args.models]
        probes += [(m, list(map(effort_setting, args.settings or ["default"]))) for m in args.models or [] if m not in dict(PROBES)]

    rows, summary, total = [], [], 0.0
    for model, settings in probes:
        for setting, effort, extra, suffix in settings:
            seen = []
            for i in range(min(args.repeats, MAX_TRIES.get(model, args.repeats))):
                rec = {"time": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "model": model, "setting": setting,
                       "reasoning_effort": effort, "extra_body": extra, "suffix": suffix, "task": args.task, "try": i}
                try:
                    r = llm_client.call_llm(model, prompt + f"\n/* call-id: {llm_client.new_call_id()} */\n" + suffix,
                                            reasoning_effort=effort, extra_body=extra)
                    usage = r["usage"]
                    rec.update(status="ok", finish_reason=r["finish_reason"], usage=usage, cost_usd=r["cost_usd"],
                               elapsed_seconds=r["elapsed_seconds"], underlying_model=r["underlying_model"],
                               reasoning_chars=len(r["reasoning"] or ""), think_in_content=bool(THINK_RE.search(r["content"])),
                               answer=r["content"][-200:])
                except LlmError as e:
                    usage = e.usage
                    rec.update(status=str(e), usage=usage, cost_usd=e.cost_usd, elapsed_seconds=e.elapsed_seconds)
                append_call(args.log, rec)
                total += rec["cost_usd"] or 0
                seen.append(rec)
                print(f"{model} {setting} #{i}: {rec['status']} {rec['elapsed_seconds']} s", file=sys.stderr)
                answer = " ".join(THINK_RE.sub("", rec.get("answer", "")).split())[-30:]
                rows.append([model, setting, i, rec["status"], rec.get("finish_reason", "-"), reasoning_tokens(usage),
                             usage.get("completion_tokens"), rec.get("reasoning_chars", "-"),
                             "yes" if rec.get("think_in_content") else "no", answer or "-", rec["elapsed_seconds"],
                             f"${rec['cost_usd'] or 0:.4f}", rec.get("underlying_model", "-")])
            ok = [r for r in seen if r["status"] == "ok"]
            mean = lambda f: f"{sum(f(r) or 0 for r in ok) / len(ok):.0f}" if ok else "-"
            summary.append([model, setting, f"{len(ok)}/{len(seen)}", mean(lambda r: reasoning_tokens(r["usage"])),
                            mean(lambda r: r["usage"].get("completion_tokens")), mean(lambda r: r["reasoning_chars"]),
                            sum(r["think_in_content"] for r in ok), max((r["elapsed_seconds"] or 0 for r in seen), default=0),
                            f"${sum(r['cost_usd'] or 0 for r in seen) / len(seen):.4f}"])

    heading("Requests" + (f" (prompt: {args.task[0]} {args.task[1]})" if args.task else f" (prompt: {QUESTION})"))
    table(["model", "setting", "try", "status", "finish", "reasoning tokens", "completion tokens",
           "reasoning_content chars", "<think> in content", "answer (end)", "s", "cost", "underlying model"], rows)
    heading("Per model and setting")
    table(["model", "setting", "answered", "reasoning tokens", "completion tokens", "reasoning_content chars",
           "answers with <think>", "slowest s", "cost per request"], summary)
    print(f"Total cost: ${total:.4f} (log: {args.log})")


if __name__ == "__main__":
    main()
