#!/usr/bin/env python3
# check the claude-cli backend against scripts/fake_claude.py without calling Claude: isolation flags and environment,
# thinking on/off, usage limits (wait and retry, no error), hangs (killed), CLI errors, then run_desc end to end
# usage: python3 scripts/test_claude_cli.py

import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from datetime import datetime

os.environ["LLM_BACKEND"] = "claude-cli"
SCRIPTS = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPTS)
FAKE = os.path.join(SCRIPTS, "fake_claude.py")
GLOBL = re.compile(r"\.globl\s+axpy\b")
WORK = tempfile.mkdtemp(prefix="claude_cli_test_")
os.environ.update(CLAUDE_CLI=FAKE, CLAUDE_CLI_CWD=os.path.join(WORK, "cwd"), CLAUDE_CLI_LIMIT_MARGIN="1",
                  CLAUDE_CLI_LIMIT_RETRY="2", ANTHROPIC_API_KEY="sk-must-not-be-passed-on")

import claude_cli
import llm_client
from claude_cli import call
from llm_client import LlmError


def report(ok, label, detail):
    print(f"{'ok ' if ok else 'BAD'} {label}: {detail}")
    return ok


def fake(mode, log, **env):
    os.environ.update(FAKE_CLAUDE_MODE=mode, FAKE_CLAUDE_LOG=log, **env)


def last_call(log):
    return json.loads(open(log).read().splitlines()[-1])


def error_of(f):
    try:
        f()
    except LlmError as e:
        return str(e)
    return None


def main():
    ok = True
    log = os.path.join(WORK, "calls.jsonl")
    prompt = "Write ... Export the function with .globl axpy.\n"

    fake("ok", log)
    r = call("claude-opus-5-5", prompt, thinking="on")
    seen = last_call(log)
    argv = seen["argv"]
    ok &= report(bool(GLOBL.search(r["content"])) and r["finish_reason"] == "end_turn", "answer", r["finish_reason"])
    ok &= report(all(f in argv for f in ("-p", "--strict-mcp-config", "--disable-slash-commands", "--no-session-persistence"))
                 and argv[argv.index("--tools") + 1] == "" and argv[argv.index("--setting-sources") + 1] == ""
                 and argv[argv.index("--system-prompt") + 1] == claude_cli.SYSTEM_PROMPT
                 and argv[argv.index("--model") + 1] == "claude-opus-5-5", "isolation flags", " ".join(argv[:12]))
    ok &= report(seen["cwd"] == os.environ["CLAUDE_CLI_CWD"] and seen["cwd_files"] == []
                 and not seen["cwd"].startswith(ROOT), "empty cwd outside the repo", seen["cwd"])
    ok &= report(seen["prompt_chars"] == len(prompt), "prompt on stdin", seen["prompt_chars"])
    ok &= report(not any(k.startswith(("ANTHROPIC", "CLAUDECODE", "CLAUDE_CODE_SESSION", "CLAUDE_CODE_CHILD")) for k in seen["env"]),
                 "ANTHROPIC_API_KEY and the calling session's variables not passed on", sorted(seen["env"]))
    ok &= report(seen["env"].get("MAX_THINKING_TOKENS") == "32000" and seen["env"].get("CLAUDE_CODE_EFFORT_LEVEL") == "medium"
                 and seen["env"].get("CLAUDE_CODE_DISABLE_ADVISOR_TOOL") == "1", "thinking on, effort pinned",
                 {k: seen["env"].get(k) for k in ("MAX_THINKING_TOKENS", "CLAUDE_CODE_EFFORT_LEVEL")})
    ok &= report(r["usage"]["completion_tokens_details"]["reasoning_tokens"] == 120 and r["usage"]["prompt_tokens"] == 605
                 and r["cost_usd"] == 0.0123 and r["record"]["billing"] == "subscription"
                 and r["record"]["thinking_blocks"] == 1 and r["record"]["rate_limit"]["five_hour"] == 0.4,
                 "usage, cost, billing, thinking blocks, window", r["record"])
    ok &= report(r["sampling_params"]["thinking"] == "on" and r["sampling_params"]["temperature"] == "cli default",
                 "sampling_params", r["sampling_params"])

    r = call("claude-opus-5", prompt, thinking="off")
    seen = last_call(log)
    ok &= report(seen["env"]["MAX_THINKING_TOKENS"] == "0" and seen["env"]["CLAUDE_CODE_EFFORT_LEVEL"] == "high"
                 and r["usage"]["completion_tokens_details"]["reasoning_tokens"] == 0 and r["record"]["thinking_blocks"] == 0,
                 "thinking off", seen["env"]["MAX_THINKING_TOKENS"])
    r = call("claude-haiku-4-5-20251001", prompt, thinking="on", effort=None)
    ok &= report("CLAUDE_CODE_EFFORT_LEVEL" not in last_call(log)["env"], "no effort for Haiku 4.5", r["sampling_params"]["effort"])

    for mode, label in (("limit", "usage limit with reset time"), ("limit-text", "usage limit as text only")):
        os.remove(log)
        fake(mode, log, FAKE_CLAUDE_LIMITS="2")
        start = time.time()
        r = call("claude-sonnet-5-5", prompt, thinking="on")
        calls = len(open(log).read().splitlines())
        ok &= report(calls == 3 and r["record"]["limit_wait_seconds"] >= 2 and GLOBL.search(r["content"])
                     and r["elapsed_seconds"] < time.time() - start, label,
                     f"{calls} calls, waited {r['record']['limit_wait_seconds']}s, elapsed {r['elapsed_seconds']}s")

    fake("ok", log)
    claude_cli.PAUSE_AT = 0.05
    for resume, label, want in ((time.time() + 2, "pause until the resume time", (2, 4)),
                                (time.time() - 60, "no pause after the resume time", (0, 0)),
                                (None, "no resume time: pause until the 7-day reset", (2, 5))):
        claude_cli.RESUME_AFTER = resume and datetime.fromtimestamp(resume).astimezone().isoformat()
        start = time.time()
        r = call("claude-sonnet-5", prompt, thinking="on")
        ok &= report(want[0] <= r["record"]["pause_seconds"] <= want[1] and time.time() - start >= want[0], label,
                     f"paused {r['record']['pause_seconds']}s")
    claude_cli.PAUSE_AT = 0.5
    r = call("claude-sonnet-5", prompt, thinking="on")
    ok &= report(r["record"]["pause_seconds"] == 0, "no pause below the threshold", r["record"]["pause_seconds"])
    claude_cli.PAUSE_AT = 0

    fake("hang", log)
    claude_cli.TIMEOUT = 3
    start = time.time()
    err = error_of(lambda: call("claude-haiku-5-5", prompt, thinking="on"))
    time.sleep(0.5)
    left = subprocess.run(["pgrep", "-f", f"{FAKE} -p"], capture_output=True, text=True).stdout.split()
    ok &= report(err and "killed" in err and time.time() - start < 10 and not left, "hang killed",
                 f"{err} after {time.time() - start:.0f}s, left {left}")
    claude_cli.TIMEOUT = 1800

    for mode, want in (("error", "API Error: 500"), ("tools", "not isolated"), ("wrong-model", "answered by")):
        fake(mode, log)
        err = error_of(lambda: call("claude-haiku-5-5", prompt, thinking="on"))
        ok &= report(err is not None and want in err, f"CLI {mode} is an LlmError", err)
    os.makedirs(claude_cli.CWD, exist_ok=True)
    open(os.path.join(claude_cli.CWD, "leak.txt"), "w").close()
    fake("ok", log)
    err = error_of(lambda: call("claude-haiku-5-5", prompt))
    ok &= report(err is not None and "not empty" in err, "refuses a non-empty cwd", err)
    os.remove(os.path.join(claude_cli.CWD, "leak.txt"))
    err = error_of(lambda: llm_client.call_llm("claude-haiku-5-5", prompt, temperature=0.0))
    ok &= report(err is not None and "temperature" in err, "temperature refused", err)

    run_id = f"test_claude_cli_{os.getpid()}"
    run_dir = os.path.join(ROOT, "results", run_id)
    os.remove(log)
    fake("limit", log, FAKE_CLAUDE_LIMITS="2")
    try:
        for thinking in ("on", "off"):
            out = subprocess.run([sys.executable, os.path.join(SCRIPTS, "run_desc.py"), "--run-id", run_id,
                                  "--models", "claude-opus-5", "--kernels", "axpy", "dot", "--levels", "L0", "L2",
                                  "--max-repairs", "1", "--feedback", "compile+tests", "--thinking", thinking,
                                  "--shard", f"opus-5-{thinking}"], capture_output=True, text=True)
            ok &= report(out.returncode == 0 and "[usage-limit]" in out.stdout if thinking == "on" else out.returncode == 0,
                         f"run_desc thinking {thinking}", out.stdout.strip().splitlines()[-3:] + out.stderr.splitlines()[-2:])
        records = [json.loads(l) for f in sorted(os.listdir(os.path.join(run_dir, "calls")))
                   for l in open(os.path.join(run_dir, "calls", f))]
        labels = sorted({r["model"] for r in records})
        ok &= report(len(records) == 8 and all(r["stage"] == "pass" and r["billing"] == "subscription"
                                               and r["base_model"] == "claude-opus-5" for r in records)
                     and labels == ["claude-opus-5@thinking-off", "claude-opus-5@thinking-on"],
                     "records", f"{len(records)} records, {labels}")
        on = next(r for r in records if r["thinking"] == "on" and r["limit_wait_seconds"])
        ok &= report(on["sampling_params"]["thinking"] == "on" and on["usage"]["completion_tokens_details"]["reasoning_tokens"] == 120,
                     "limit wait logged, not an error", f"waited {on['limit_wait_seconds']}s, stage {on['stage']}")
        run_txt = open(os.path.join(run_dir, "RUN.txt")).read()
        ok &= report("billing: subscription" in run_txt and "cannot be set" in run_txt and "2.1.312 (fake)" in run_txt,
                     "RUN.txt", next(l for l in run_txt.splitlines() if l.startswith("backend:")))
        out = subprocess.run([sys.executable, os.path.join(SCRIPTS, "desc_thinking.py"), run_dir],
                             capture_output=True, text=True).stdout
        ok &= report("| claude-opus-5@thinking-off (" in out and "+0 pts" in out, "desc_thinking pairs off/on",
                     next((l for l in out.splitlines() if l.startswith("| claude-opus-5@thinking-off (") and "| all |" in l), out[-300:]))

        fake("hang", log)
        env = dict(os.environ, CLAUDE_CLI_TIMEOUT="2")
        out = subprocess.run([sys.executable, os.path.join(SCRIPTS, "gen_desc.py"), "--kernel", "axpy", "--level", "L1",
                              "--model", "claude-haiku-5-5", "--thinking", "on", "--run-dir", run_dir], env=env,
                             capture_output=True, text=True)
        rec = json.loads(open(os.path.join(run_dir, "calls.jsonl")).read().splitlines()[-1])
        ok &= report(rec["stage"] == "no_answer" and rec["stop_reason"] == "api_error" and "killed" in rec["error"],
                     "hang in gen_desc is an api_error", rec["error"])
    finally:
        shutil.rmtree(run_dir, ignore_errors=True)
        shutil.rmtree(WORK, ignore_errors=True)

    print("\nall good" if ok else "\nproblems found")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
