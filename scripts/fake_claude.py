#!/usr/bin/env python3
# stands in for the claude CLI in tests (CLAUDE_CLI=scripts/fake_claude.py): canned stream-json, answers with the
# kernel's hand.S; FAKE_CLAUDE_MODE ok | limit (first FAKE_CLAUDE_LIMITS calls hit a usage limit, then ok) |
# limit-text (limit reported only as text) | hang | error | tools | wrong-model
# FAKE_CLAUDE_LOG gets one line per call with argv, cwd, files in cwd, the names of CLAUDE*/ANTHROPIC* environment
# variables and the values of the switches the backend sets

import json
import os
import re
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHOWN = ("CLAUDE_CODE_DISABLE", "CLAUDE_CODE_NO_", "CLAUDE_CODE_EFFORT", "CLAUDE_CODE_MAX_OUTPUT", "MAX_THINKING")


def emit(event):
    print(json.dumps(event), flush=True)


def calls_so_far(log):
    return sum(1 for _ in open(log)) if log and os.path.exists(log) else 0


def main():
    if "--version" in sys.argv:
        print("2.1.312 (fake)")
        return
    mode = os.environ.get("FAKE_CLAUDE_MODE", "ok")
    log = os.environ.get("FAKE_CLAUDE_LOG")
    model = sys.argv[sys.argv.index("--model") + 1]
    prompt = sys.stdin.read()
    n = calls_so_far(log)
    if log:
        with open(log, "a") as f:
            f.write(json.dumps({"argv": sys.argv[1:], "cwd": os.getcwd(), "cwd_files": os.listdir("."),
                                "env": {k: v if k.startswith(SHOWN) else "set" for k, v in os.environ.items()
                                        if k.startswith(("ANTHROPIC", "CLAUDE", "MAX_THINKING"))},
                                "prompt_chars": len(prompt)}) + "\n")
    tools = ["Read"] if mode == "tools" else []
    emit({"type": "system", "subtype": "init", "tools": tools, "mcp_servers": [], "model": model, "apiKeySource": "none"})
    if mode == "hang":
        time.sleep(3600)
    limited = mode in ("limit", "limit-text") and n < int(os.environ.get("FAKE_CLAUDE_LIMITS", "1"))
    if mode != "limit-text":
        emit({"type": "rate_limit_event", "rate_limit_info": {
            "status": "rejected" if limited else "allowed", "resetsAt": int(time.time()) + 2, "rateLimitType": "five_hour",
            "unifiedWindows": {"five_hour": {"utilization": 1.0 if limited else 0.4},
                               "seven_day": {"utilization": 0.1, "resetsAt": int(time.time()) + 3}}}})
    if limited:
        emit({"type": "result", "subtype": "success", "is_error": True, "api_error_status": None,
              "result": "You've hit your session limit · resets 3am", "total_cost_usd": 0, "usage": {}})
        sys.exit(1)
    if mode == "error":
        emit({"type": "result", "subtype": "error_during_execution", "is_error": True, "api_error_status": 500,
              "result": "API Error: 500 internal server error", "total_cost_usd": 0, "usage": {}})
        sys.exit(1)
    name = re.search(r"\.globl (\w+)", prompt)
    hand = os.path.join(ROOT, "kernels", "desc", name.group(1) if name else "-", "hand.S")
    asm = open(hand).read() if os.path.exists(hand) else ".globl f\nf:\n    ret\n"
    thinking = 0 if os.environ.get("MAX_THINKING_TOKENS") == "0" else 120
    content = ([{"type": "thinking", "thinking": "", "signature": "x"}] if thinking else []) + \
              [{"type": "text", "text": f"```asm\n{asm}```"}]
    emit({"type": "assistant", "message": {"model": model, "content": content}})
    answered = "claude-other" if mode == "wrong-model" else model
    emit({"type": "result", "subtype": "success", "is_error": False, "stop_reason": "end_turn",
          "result": f"```asm\n{asm}```", "total_cost_usd": 0.0123, "num_turns": 1,
          "usage": {"input_tokens": 5, "cache_creation_input_tokens": 600, "cache_read_input_tokens": 0,
                    "output_tokens": 300 + thinking, "output_tokens_details": {"thinking_tokens": thinking}},
          "modelUsage": {answered: {"inputTokens": 5, "outputTokens": 300 + thinking}}})


if __name__ == "__main__":
    main()
