# ask an LLM to replace a kernel's #pragma scop region with 8086 asm
# usage:python3 generate_asm.py --kernel atax --src atax.c --out out.c --alias RiVault/Instruction-Medium

import argparse, json, os, random, re, sys, time, urllib.request

BASE_URL = os.environ.get("RIVAULT_BASE_URL", "https://api.class2.llm.ai.r-ccs.riken.jp/v1")
API_KEY = os.environ.get("RIVAULT_API_KEY")
HERE = os.path.dirname(os.path.abspath(__file__))
PROMPT_FILE = os.path.join(HERE, "..", "prompts", "i8086_prompt_template.txt")

p = argparse.ArgumentParser()
p.add_argument("--kernel", required=True)
p.add_argument("--src", required=True)
p.add_argument("--out", required=True)
p.add_argument("--alias", default="RiVault/Instruction-Medium")
p.add_argument("--dry-run", action="store_true")
p.add_argument("--log", default="results/llm_calls.jsonl")
p.add_argument("--max-tokens", type=int, default=None, help="omit for no cap")
p.add_argument("--prompt-template", default=PROMPT_FILE)
args = p.parse_args()

src = open(args.src).read()

# pull out the #pragma scop ... #pragma endscop block
m = re.search(r"(#pragma scop\s*\n)(.*?)(\n\s*#pragma endscop)", src, re.DOTALL)
if not m:
    sys.exit("no #pragma scop/endscop block found in " + args.src)
before, region, after = src[:m.start(2)], m.group(2), src[m.end(2):]

reasoning_file = None
usage = {}
cost = None
key_spend = None

if args.dry_run:
    print(f"[{args.kernel}] dry run, found region ({len(region)} chars), not calling the API")
    replacement = "  /* DRY RUN PLACEHOLDER */\n" + region
    model = "dry-run"
    elapsed = 0
else:
    if not API_KEY:
        sys.exit("set RIVAULT_API_KEY first")

    # nonce + random seed so repeated calls aren't served from a cach
    nonce = f"{random.randint(0, 999999):06d}"
    prompt = open(args.prompt_template).read().format(kernel_source=src) + f"\n/* call-id: {nonce} */\n"
    seed = random.randint(1, 2**31 - 1)

    body_dict = {"model": args.alias, "messages": [{"role": "user", "content": prompt}], "seed": seed}
    if args.max_tokens is not None:
        body_dict["max_tokens"] = args.max_tokens
    body = json.dumps(body_dict).encode()
    req = urllib.request.Request(
        f"{BASE_URL}/chat/completions", data=body,
        headers={"Authorization": f"Bearer {API_KEY}", "Content-Type": "application/json"},
    )
    print(f"[{args.kernel}] calling {args.alias} (max_tokens={args.max_tokens or 'none'}) ...")
    start = time.time()
    resp = None
    cost = None
    key_spend = None

    def to_float(x):
        try:
            return float(x)
        except (TypeError, ValueError):
            return None

    try:
        http_resp = urllib.request.urlopen(req, timeout=900)
        cost = to_float(http_resp.headers.get("X-Litellm-Response-Cost"))
        key_spend = to_float(http_resp.headers.get("X-Litellm-Key-Spend"))
        resolved_model = http_resp.headers.get("X-Litellm-Model-Name")
        resp = json.loads(http_resp.read())
        elapsed = time.time() - start
        message = resp["choices"][0]["message"]
        content = message["content"]
        finish_reason = resp["choices"][0].get("finish_reason", "unknown")
        if content is None:
            raise ValueError(f"empty content, finish_reason={finish_reason} (likely hit max_tokens before finishing)")
        replacement = content.strip()
        replacement = re.sub(r"^```[a-z]*\n|\n```$", "", replacement)
        model = resolved_model or resp.get("model", "unknown")
        usage = resp.get("usage", {})

        # save the reasoning trace separately since potentially very long
        reasoning = message.get("reasoning_content")
        if reasoning:
            slug = args.alias.replace("/", "_")
            reasoning_dir = os.path.join(os.path.dirname(args.log) or ".", "reasoning")
            os.makedirs(reasoning_dir, exist_ok=True)
            reasoning_file = os.path.join(reasoning_dir, f"{args.kernel}__{slug}.txt")
            open(reasoning_file, "w").write(reasoning)

        cost_str = f"${cost:.6f}" if cost is not None else "$?"
        print(f"[{args.kernel}] underlying model: {model}, took {elapsed:.1f}s, usage: {usage}, cost: {cost_str}" + (f", reasoning saved to {reasoning_file}" if reasoning_file else ""))
    except Exception as e:
        elapsed = time.time() - start
        if cost is None and hasattr(e, "headers"):
            cost = to_float(e.headers.get("X-Litellm-Response-Cost"))
            key_spend = to_float(e.headers.get("X-Litellm-Key-Spend"))
        # actual tokens the API reports using, vs what we asked for
        usage = resp.get("usage", {}) if resp else {}
        cost_str = f"${cost:.6f}" if cost is not None else "$?"
        print(f"[{args.kernel}] call failed after {elapsed:.1f}s: {e}"
              + (f" -- usage: {usage}, cost: {cost_str}" if usage or cost else ""))
        os.makedirs(os.path.dirname(args.log) or ".", exist_ok=True)
        with open(args.log, "a") as f:
            f.write(json.dumps({"kernel": args.kernel, "alias_requested": args.alias,
                                 "underlying_model": f"error: {e}", "dry_run": args.dry_run,
                                 "elapsed_seconds": round(elapsed, 1), "reasoning_file": None,
                                 "usage": usage, "max_tokens_requested": args.max_tokens,
                                 "cost_usd": cost, "key_spend_total_usd": key_spend,
                                 "prompt_template": args.prompt_template}) + "\n")
        sys.exit(1)

os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
open(args.out, "w").write(before + replacement + after)
print(f"[{args.kernel}] wrote {args.out}")

os.makedirs(os.path.dirname(args.log) or ".", exist_ok=True)
with open(args.log, "a") as f:
    f.write(json.dumps({"kernel": args.kernel, "alias_requested": args.alias,
                         "underlying_model": model, "dry_run": args.dry_run,
                         "elapsed_seconds": round(elapsed, 1),
                         "reasoning_file": reasoning_file,
                         "max_tokens_requested": args.max_tokens,
                         "usage": usage,
                         "cost_usd": cost, "key_spend_total_usd": key_spend,
                         "prompt_template": args.prompt_template}) + "\n")