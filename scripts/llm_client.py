#!/usr/bin/env python3
# shared API / splice / log helpers for the LLM-asm benchmark pipeline.
# Extracted from generate_asm.py so the single-shot flow and the new repair
# orchestrator (generate_with_repair.py) share one implementation.

import json
import os
import random
import re
import time
import urllib.error
import urllib.request

BASE_URL = os.environ.get("RIVAULT_BASE_URL", "https://api.class2.llm.ai.r-ccs.riken.jp/v1")
API_KEY = os.environ.get("RIVAULT_API_KEY")
REQUEST_TIMEOUT = 900

# Retry policy for transient gateway failures (504/502/503/429, timeout, conn reset).
# These were the dominant api_error cause in the 2026-09-28 runs (Reasoning-Small /
# Instruction-Tiny 504s on generate turns). Hard errors (auth 401/403, config 4xx/5xx
# bodies) and empty-content responses are NOT retried here (content=None is a caller decision).
MAX_RETRIES = int(os.environ.get("RIVAULT_RETRIES", "2"))
RETRYABLE_STATUS = frozenset({429, 500, 502, 503, 504})
RETRY_BACKOFF_BASE = 2.0      # seconds, then doubled per attempt
RETRY_BACKOFF_MAX = 12.0


def _backoff(attempt):
    return min(RETRY_BACKOFF_BASE * (2 ** attempt), RETRY_BACKOFF_MAX) \
        + random.uniform(0, 0.5)

SCOP_RE = re.compile(r"(#pragma scop\s*\n)(.*?)(\n\s*#pragma endscop)", re.DOTALL)


class LlmError(Exception):
    """A failed chat/completions call (network, HTTP status, empty content...).
    Carries what the gateway reported so callers can log it like generate_asm.py did."""

    def __init__(self, message, *, usage=None, cost_usd=None, key_spend=None,
                 elapsed_seconds=None):
        super().__init__(message)
        self.usage = usage or {}
        self.cost_usd = cost_usd
        self.key_spend = key_spend
        self.elapsed_seconds = elapsed_seconds


def split_scop(src):
    """Split a kernel source into (before, region, after) around #pragma scop/endscop."""
    m = SCOP_RE.search(src)
    if not m:
        raise ValueError("no #pragma scop/endscop block found in source")
    before, region, after = src[:m.start(2)], m.group(2), src[m.end(2):]
    return before, region, after


def splice(before, replacement, after):
    return before + replacement + after


def new_call_id():
    """Anti-caching nonce, same as generate_asm.py's random 6-digit id."""
    return f"{random.randint(0, 999999):06d}"


def build_prompt(template_path, kernel_source, call_id=None):
    """Fill a prompt template with the kernel source and an optional anti-cache nonce."""
    template = open(template_path).read()
    prompt = template.format(kernel_source=kernel_source)
    if call_id:
        prompt += f"\n/* call-id: {call_id} */\n"
    return prompt


def strip_fences(content):
    """generate_asm.py's original post-processing: strip leading/trailing markdown fences."""
    return re.sub(r"^```[a-z]*\n|\n```$", "", content.strip())


def call_llm(alias, prompt, *, max_tokens=None, timeout=REQUEST_TIMEOUT,
             base_url=None, api_key=None):
    """One chat/completions round trip with a fresh random seed, retrying transient
    gateway failures (504/502/503/429/timeouts) with exponential backoff.

    Returns dict(content, underlying_model, usage, cost_usd, key_spend_total_usd,
                 finish_reason, elapsed_seconds, reasoning).
    Raises LlmError on failure (HTTP errors carry usage/cost read from the gateway).
    content=None is reported as LlmError ("empty content ...") and is NOT retried here;
    callers decide whether raising the token cap makes sense (see generate_with_repair)."""
    base_url = (base_url or BASE_URL).rstrip("/")
    api_key = api_key if api_key is not None else API_KEY
    if not api_key:
        raise LlmError("set RIVAULT_API_KEY first")

    seed = random.randint(1, 2**31 - 1)
    body_dict = {"model": alias, "messages": [{"role": "user", "content": prompt}], "seed": seed}
    if max_tokens is not None:
        body_dict["max_tokens"] = max_tokens
    body = json.dumps(body_dict).encode()
    req = urllib.request.Request(
        f"{base_url}/chat/completions", data=body,
        headers={"Authorization": f"Bearer {api_key}", "Content-Type": "application/json"},
    )
    start = time.time()

    def to_float(x):
        try:
            return float(x)
        except (TypeError, ValueError):
            return None

    def run_once(attempt):
        """Single HTTP round trip. Returns (resp_json, http_status, cost, key_spend,
        resolved_model) or raises LlmError for non-retryable / exhausted failures."""
        try:
            http_resp = urllib.request.urlopen(req, timeout=timeout)
        except urllib.error.HTTPError as e:
            status = e.code
            headers = getattr(e, "headers", None)
            cost = to_float(headers.get("X-Litellm-Response-Cost")) if headers else None
            key_spend = to_float(headers.get("X-Litellm-Key-Spend")) if headers else None
            try:
                usage = json.loads(e.read().decode("utf-8", "replace") or "{}").get("usage", {})
            except Exception:
                usage = {}
            if status in RETRYABLE_STATUS and attempt < MAX_RETRIES:
                # let the retry loop handle it; carry cost so it's not lost on success
                retryable = LlmError(str(e), usage=usage, cost_usd=cost,
                                     key_spend=key_spend)
                raise _TransientError(retryable) from e
            raise LlmError(f"HTTP {status}: {e}", usage=usage, cost_usd=cost,
                           key_spend=key_spend,
                           elapsed_seconds=round(time.time() - start, 1)) from e
        except Exception as e:   # urllib.error.URLError (timeout/conn), socket errors...
            if attempt < MAX_RETRIES:
                # timeout / conn reset are transient -> retry; re-raise type via helper
                raise _TransientError(LlmError(str(e), elapsed_seconds=round(time.time() - start, 1))) from e
            raise LlmError(str(e), elapsed_seconds=round(time.time() - start, 1)) from e

        cost = to_float(http_resp.headers.get("X-Litellm-Response-Cost"))
        key_spend = to_float(http_resp.headers.get("X-Litellm-Key-Spend"))
        resolved_model = http_resp.headers.get("X-Litellm-Model-Name")
        try:
            resp = json.loads(http_resp.read())
        except Exception as e:
            if attempt < MAX_RETRIES:
                raise _TransientError(LlmError(str(e),
                                               elapsed_seconds=round(time.time() - start, 1))) from e
            raise LlmError(str(e), elapsed_seconds=round(time.time() - start, 1)) from e
        return resp, http_resp.status, cost, key_spend, resolved_model

    last_err = None
    resp = None
    cost = key_spend = resolved_model = None
    for attempt in range(MAX_RETRIES + 1):
        try:
            resp, status, cost, key_spend, resolved_model = run_once(attempt)
            break
        except _TransientError as e:
            last_err = e.err
            if attempt < MAX_RETRIES:
                delay = _backoff(attempt)
                print(f"[retry] {alias} transient failure (attempt {attempt + 1}), "
                      f"sleeping {delay:.1f}s: {e.err}")
                time.sleep(delay)
            # else: fall through and raise last_err after the loop
    if resp is None:
        raise last_err

    elapsed = round(time.time() - start, 1)
    message = resp["choices"][0]["message"]
    content = message["content"]
    finish_reason = resp["choices"][0].get("finish_reason", "unknown")
    if content is None:
        raise LlmError(f"empty content, finish_reason={finish_reason} "
                       "(likely hit max_tokens before finishing)",
                       usage=resp.get("usage", {}), cost_usd=cost,
                       key_spend=key_spend, elapsed_seconds=elapsed)
    return {
        "content": content,
        "underlying_model": resolved_model or resp.get("model", "unknown"),
        "usage": resp.get("usage", {}),
        "cost_usd": cost,
        "key_spend_total_usd": key_spend,
        "finish_reason": finish_reason,
        "elapsed_seconds": elapsed,
        "reasoning": message.get("reasoning_content"),
    }


class _TransientError(Exception):
    """Internal: a retryable gateway failure wrapped so the retry loop can catch it
    separately from a terminal LlmError. Carries the underlying LlmError."""
    def __init__(self, err):
        super().__init__(str(err))
        self.err = err


# --- region salvage: turn a messy model answer into a usable #pragma scop replacement ---

THINK_RE = re.compile(r"<\s*/?\s*(?:\w+:)?think(?:ing)?\s*>", re.IGNORECASE)
ASM_KW_RE = re.compile(
    r"(?m)^\s*(?:__asm__(?:\s*\(\s*(?:__)?volatile\s*\))?|__asm\b|_asm\b|asm\b)")


def _asm_block(text):
    """Extract a single inline asm() statement, skipping parens inside strings/comments."""
    m = ASM_KW_RE.search(text)
    if not m:
        return None
    i = text.find("(", m.end())
    if i == -1:
        return None
    depth, j = 0, i
    n = len(text)
    in_str = in_line = in_block = False
    while j < n:
        c = text[j]
        if in_str:
            if c == "\\":
                j += 2
                continue
            if c == '"':
                in_str = False
        elif in_line:
            if c == "\n":
                in_line = False
        elif in_block:
            if c == "*" and j + 1 < n and text[j + 1] == "/":
                in_block = False
                j += 1
        else:
            if c == '"':
                in_str = True
            elif c == "/" and j + 1 < n and text[j + 1] == "/":
                in_line = True
                j += 1
            elif c == "/" and j + 1 < n and text[j + 1] == "*":
                in_block = True
                j += 1
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
                if depth == 0:
                    end = j + 1
                    while end < n and text[end] in " \t":
                        end += 1
                    if end < n and text[end] == ";":
                        end += 1
                    return text[m.start():end].strip()
        j += 1
    return None


def clean_region(content):
    """Salvage a usable #pragma scop region from a model reply, or None if none exists.

    Strips markdown fences and thinking tags; when the model returned a whole file it
    re-extracts inside #pragma scop; otherwise isolates a single inline asm() block.
    Called on repair replies (and initial replies too); the single-shot faithfull path
    (generate_asm.py) keeps its original strip_fences behaviour."""
    if content is None:
        return None
    text = content.strip()
    text = re.sub(r"^```[a-z]*\s*\n?", "", text)
    text = re.sub(r"\n?\s*```\s*$", "", text)
    text = THINK_RE.sub("", text)
    if "#pragma scop" in text:
        m = re.search(r"(?m)^#pragma scop[^\n]*\n(.*?)(?:(?:\n\s*#pragma endscop)|\Z)",
                      text, re.DOTALL)
        if m:
            return m.group(1).strip()
    block = _asm_block(text)
    return block.strip() if block else None


def save_reasoning(log_path, kernel, slug, tag, reasoning):
    """Write a reasoning trace next to <log_path> under reasoning/.

    tag '' for generation -> <kernel>__<slug>.txt; 'repair1' -> <kernel>__<slug>_repair1.txt.
    Returns the path written, or None when there is no reasoning to save."""
    if not reasoning:
        return None
    reasoning_dir = os.path.join(os.path.dirname(log_path) or ".", "reasoning")
    os.makedirs(reasoning_dir, exist_ok=True)
    name = f"{kernel}__{slug}" + (f"_{tag}" if tag else "") + ".txt"
    path = os.path.join(reasoning_dir, name)
    with open(path, "w") as f:
        f.write(reasoning)
    return path


def append_call(log_path, fields):
    """Append one call record to an llm_calls.jsonl log."""
    os.makedirs(os.path.dirname(log_path) or ".", exist_ok=True)
    with open(log_path, "a") as f:
        f.write(json.dumps(fields) + "\n")
