#!/usr/bin/env python3
# cost of a model's calls computed from logged token counts, for APIs that send no cost header (e.g. OpenAI)
# usage: python3 scripts/token_cost.py results/desc_full <model> <usd per 1M input tokens> <usd per 1M output tokens>

import sys

from gen_desc import load_records
from md import heading, table

run_dir, model, price_in, price_out = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4])
records = [r for r in load_records(run_dir) if r["model"] == model]
tokens_in = sum((r.get("usage") or {}).get("prompt_tokens") or 0 for r in records)
tokens_out = sum((r.get("usage") or {}).get("completion_tokens") or 0 for r in records)
cells = {r["cell"] for r in records if r.get("stop_reason")}
cost = (tokens_in * price_in + tokens_out * price_out) / 1e6

heading(f"Token cost of {model} in {run_dir}")
table(["calls", "finished cells", "input tokens", "output tokens", "cost", "cost per cell"],
      [[len(records), len(cells), tokens_in, tokens_out, f"${cost:.2f}", f"${cost / len(cells):.4f}" if cells else "-"]])
