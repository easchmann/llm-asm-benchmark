# ask an LLM to replace a kernel's #pragma scop region with 8086 asm
# usage:python3 generate_asm.py --kernel atax --src atax.c --out out.c --alias RiVault/Instruction-Medium

import argparse
import json
import os
import sys

from llm_client import (API_KEY, LlmError, build_prompt, call_llm, new_call_id,
                        save_reasoning, split_scop, splice, strip_fences)

HERE = os.path.dirname(os.path.abspath(__file__))
PROMPT_FILE = os.path.join(HERE, "..", "prompts", "i8086_prompt_template.txt")


def main():
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
    try:
        before, region, after = split_scop(src)
    except ValueError as e:
        sys.exit(str(e))

    usage = {}
    cost = None
    key_spend = None
    reasoning_file = None

    if args.dry_run:
        print(f"[{args.kernel}] dry run, found region ({len(region)} chars), not calling the API")
        replacement = "  /* DRY RUN PLACEHOLDER */\n" + region
        model = "dry-run"
        elapsed = 0
    else:
        if not API_KEY:
            sys.exit("set RIVAULT_API_KEY first")

        # nonce + random seed so repeated calls aren't served from a cache
        call_id = new_call_id()
        prompt = build_prompt(args.prompt_template, src, call_id)

        print(f"[{args.kernel}] calling {args.alias} (max_tokens={args.max_tokens or 'none'}) ...")
        try:
            r = call_llm(args.alias, prompt, max_tokens=args.max_tokens)
            replacement = strip_fences(r["content"])
            model = r["underlying_model"]
            usage = r["usage"]
            cost = r["cost_usd"]
            key_spend = r["key_spend_total_usd"]
            elapsed = r["elapsed_seconds"]
            reasoning_file = save_reasoning(args.log, args.kernel,
                                            args.alias.replace("/", "_"), "", r["reasoning"])

            cost_str = f"${cost:.6f}" if cost is not None else "$?"
            print(f"[{args.kernel}] underlying model: {model}, took {elapsed:.1f}s, "
                  f"usage: {usage}, cost: {cost_str}"
                  + (f", reasoning saved to {reasoning_file}" if reasoning_file else ""))
        except LlmError as e:
            elapsed = e.elapsed_seconds or 0
            usage = e.usage
            cost = e.cost_usd
            key_spend = e.key_spend
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
    open(args.out, "w").write(splice(before, replacement, after))
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


if __name__ == "__main__":
    main()
