#!/usr/bin/env python3
# llm_client backend that asks a Claude model through the `claude` CLI and the Claude subscription (LLM_BACKEND=claude-cli)
# one `claude -p` call per prompt: no tools, no MCP servers, no settings, CLAUDE.md or memory, in an empty directory,
# with a neutral system prompt; the CLI still adds its identity line and an environment note (see RUN.txt)
# thinking off sets MAX_THINKING_TOKENS=0, which the 5.5 models ignore; on gives a 32k budget to models without
# adaptive thinking; effort is pinned to the model's CLI default unless reasoning_effort names one
# a usage limit is not an error: the call sleeps until the reset the CLI reports (or CLAUDE_CLI_LIMIT_RETRY s) and retries
# CLAUDE_CLI_PAUSE_AT=0.8 pauses after a call once the 7-day window is 80% used, until CLAUDE_CLI_RESUME_AFTER
# (e.g. 2026-10-30T18:00+09:00; no more pauses after that time; without it until the window's reset)
# usage: python3 scripts/claude_cli.py claude-haiku-5-5 [off|on] < prompt.txt

import json
import os
import re
import signal
import subprocess
import sys
import time
from datetime import datetime

from llm_client import LlmError, _clip_error

CLAUDE = os.environ.get("CLAUDE_CLI", "claude")
CWD = os.environ.get("CLAUDE_CLI_CWD", "/tmp/llm-asm-claude-cwd")
TIMEOUT = int(os.environ.get("CLAUDE_CLI_TIMEOUT", "1800"))
LIMIT_RETRY = int(os.environ.get("CLAUDE_CLI_LIMIT_RETRY", "1800"))
LIMIT_MARGIN = int(os.environ.get("CLAUDE_CLI_LIMIT_MARGIN", "120"))
PAUSE_AT = float(os.environ.get("CLAUDE_CLI_PAUSE_AT", "0"))
RESUME_AFTER = os.environ.get("CLAUDE_CLI_RESUME_AFTER")
SYSTEM_PROMPT = "You are a helpful assistant. You have no tools and cannot run commands or read files; answer with text only."
MAX_OUTPUT_TOKENS = 128000
THINKING_BUDGET = 32000
DEFAULT_EFFORT = {"claude-opus-5-5": "medium", "claude-sonnet-5-5": "medium", "claude-haiku-5-5": "medium",
                  "claude-fable-5-1": "high", "claude-opus-5": "high", "claude-sonnet-5": "high"}
FLAGS = ["-p", "--output-format", "stream-json", "--verbose", "--tools", "", "--strict-mcp-config",
         "--mcp-config", '{"mcpServers":{}}', "--setting-sources", "", "--disable-slash-commands",
         "--no-session-persistence", "--system-prompt", SYSTEM_PROMPT]
ENV = {"CLAUDE_CODE_MAX_OUTPUT_TOKENS": str(MAX_OUTPUT_TOKENS), "CLAUDE_CODE_DISABLE_ADVISOR_TOOL": "1",
       "CLAUDE_CODE_DISABLE_CLAUDE_MDS": "1", "CLAUDE_CODE_DISABLE_AUTO_MEMORY": "1",
       "CLAUDE_CODE_DISABLE_ATTACHMENTS": "1", "CLAUDE_CODE_NO_MODEL_FALLBACK": "1",
       "CLAUDE_CODE_DISABLE_MODEL_ACCESS_FALLBACK": "1", "CLAUDE_CODE_DISABLE_REFUSAL_FALLBACK": "1",
       "CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC": "1"}
KEEP_ENV = ("CLAUDE_CODE_OAUTH_TOKEN", "CLAUDE_CONFIG_DIR")
ADDED_BY_CLI = ("system prompt: a billing header line, 'You are a Claude agent, built on Anthropic's Claude Agent SDK.', "
                "then SYSTEM_PROMPT; after the prompt (before it on models older than 5.x) an environment note with the "
                "working directory, platform, shell, OS version, one sentence on treating downloaded files as untrusted, "
                "'You are powered by the model named ...', the knowledge cutoff and today's date; and the account e-mail "
                "address. Only CLAUDE_CODE_SIMPLE removes the environment note, and it disables the subscription login")
LIMIT_RE = re.compile(r"hit your .*limit|reached your .*limit|out of usage|usage limit", re.I)


class UsageLimit(Exception):
    def __init__(self, message, resets_at=None):
        super().__init__(message)
        self.resets_at = resets_at


def version():
    return subprocess.run([CLAUDE, "--version"], capture_output=True, text=True).stdout.strip()


def settings(model, thinking, effort):
    env = {**ENV, "MAX_THINKING_TOKENS": "0" if thinking == "off" else str(THINKING_BUDGET)}
    effort = effort or DEFAULT_EFFORT.get(model)
    if effort:
        env["CLAUDE_CODE_EFFORT_LEVEL"] = effort
    return env, {"thinking": thinking or "on", "effort": effort or "none",
                 "max_thinking_tokens": int(env["MAX_THINKING_TOKENS"]), "max_output_tokens": MAX_OUTPUT_TOKENS,
                 "temperature": "cli default", "seed": "none", "billing": "subscription"}


def run_cli(model, prompt, env):
    os.makedirs(CWD, exist_ok=True)
    if os.listdir(CWD):
        raise LlmError(f"{CWD} is not empty")
    full_env = {k: v for k, v in os.environ.items()
                if k in KEEP_ENV or not (k.startswith(("CLAUDE", "ANTHROPIC")) or k == "MAX_THINKING_TOKENS")} | env
    proc = subprocess.Popen([CLAUDE, *FLAGS, "--model", model], cwd=CWD, env=full_env, text=True,
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            start_new_session=True)
    try:
        out, err = proc.communicate(prompt, timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        proc.communicate()
        raise LlmError(f"claude CLI did not answer within {TIMEOUT}s, killed")
    return proc.returncode, out, err


def parse(model, returncode, out, err, elapsed):
    events = []
    for line in out.splitlines():
        try:
            events.append(json.loads(line))
        except ValueError:
            pass
    init = next((e for e in events if e.get("type") == "system" and e.get("subtype") == "init"), {})
    result = next((e for e in reversed(events) if e.get("type") == "result"), None)
    limit = next((e["rate_limit_info"] for e in reversed(events) if e.get("type") == "rate_limit_event"), {})
    if result is None:
        raise LlmError(f"claude CLI exit {returncode} without a result: {(err or out)[-400:]}", elapsed_seconds=elapsed)
    usage = result.get("usage") or {}
    thinking_tokens = (usage.get("output_tokens_details") or {}).get("thinking_tokens") or 0
    usage = {"prompt_tokens": sum(usage.get(k) or 0 for k in ("input_tokens", "cache_creation_input_tokens",
                                                               "cache_read_input_tokens")),
             "completion_tokens": usage.get("output_tokens") or 0,
             "completion_tokens_details": {"reasoning_tokens": thinking_tokens},
             **{k: usage.get(k) or 0 for k in ("input_tokens", "cache_creation_input_tokens",
                                                "cache_read_input_tokens", "output_tokens")}}
    if result.get("is_error"):
        text = str(result.get("result") or result.get("errors") or result.get("subtype"))
        if limit.get("status") == "rejected" or result.get("api_error_status") == 429 or LIMIT_RE.search(text):
            raise UsageLimit(text, limit.get("resetsAt"))
        raise LlmError(f"claude CLI: {text}", usage=usage, cost_usd=result.get("total_cost_usd"),
                       elapsed_seconds=elapsed)
    if init.get("tools") or init.get("mcp_servers") or init.get("apiKeySource", "none") != "none":
        raise LlmError(f"claude CLI was not isolated: tools={init.get('tools')} mcp={init.get('mcp_servers')} "
                       f"apiKeySource={init.get('apiKeySource')}")
    answered = sorted(result.get("modelUsage") or {})
    if answered and model not in answered:
        raise LlmError(f"asked {model}, answered by {answered}", usage=usage, cost_usd=result.get("total_cost_usd"),
                       elapsed_seconds=elapsed)
    text = result.get("result")
    if not text:
        raise LlmError(f"empty content, finish_reason={result.get('stop_reason')}", usage=usage,
                       cost_usd=result.get("total_cost_usd"), elapsed_seconds=elapsed)
    blocks = sum(b.get("type") in ("thinking", "redacted_thinking") for e in events if e.get("type") == "assistant"
                 for b in e["message"]["content"])
    return {"content": text, "underlying_model": ",".join(answered) or init.get("model") or model,
            "usage": usage, "cost_usd": result.get("total_cost_usd"), "finish_reason": result.get("stop_reason"),
            "elapsed_seconds": elapsed, "attempts": 1, "reasoning": None,
            "record": {"billing": "subscription", "thinking_blocks": blocks,
                       "rate_limit": {k: limit.get(k) for k in ("status", "rateLimitType", "utilization", "resetsAt")
                                      if k in limit} | {w: v.get("utilization") for w, v in
                                                        (limit.get("unifiedWindows") or {}).items()}
                                     | {"seven_day_resets": (limit.get("unifiedWindows") or {}).get("seven_day", {}).get("resetsAt")}}}


def call(model, prompt, *, thinking=None, effort=None):
    env, sampling = settings(model, thinking, effort)
    waited = 0
    while True:
        start = time.time()
        returncode, out, err = run_cli(model, prompt, env)
        try:
            r = parse(model, returncode, out, err, round(time.time() - start, 1))
            break
        except UsageLimit as e:
            wait = max(e.resets_at - time.time(), 0) + LIMIT_MARGIN if e.resets_at else LIMIT_RETRY
            until = time.strftime("%m-%d %H:%M", time.localtime(time.time() + wait))
            print(f"[usage-limit] {time.strftime('%m-%d %H:%M')} {model}: {_clip_error(e)}; sleeping {wait:.0f}s "
                  f"until {until}", flush=True)
            time.sleep(wait)
            waited += wait
    r["sampling_params"] = sampling
    r["record"]["limit_wait_seconds"] = round(waited)
    r["record"]["pause_seconds"] = pause(model, r["record"]["rate_limit"])
    return r


def pause(model, window):
    used = window.get("seven_day")
    if not PAUSE_AT or used is None or used < PAUSE_AT:
        return 0
    resume = datetime.fromisoformat(RESUME_AFTER).timestamp() if RESUME_AFTER else window.get("seven_day_resets") or 0
    wait = resume - time.time()
    if wait <= 0:
        return 0
    print(f"[usage-pause] {time.strftime('%m-%d %H:%M')} {model}: 7-day window {used:.0%} used, pausing until "
          f"{time.strftime('%m-%d %H:%M', time.localtime(resume))}", flush=True)
    time.sleep(wait)
    return round(wait)


if __name__ == "__main__":
    r = call(sys.argv[1], sys.stdin.read(), thinking=sys.argv[2] if len(sys.argv) > 2 else None)
    print(json.dumps({k: v for k, v in r.items() if k != "content"}, indent=1))
    print(r["content"])
