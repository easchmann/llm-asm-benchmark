# summarize results/comparison.csv + results/llm_calls.jsonl into a per-model table

import csv, json
from collections import defaultdict

rows = list(csv.DictReader(open("results/comparison.csv")))

times = defaultdict(list)
tokens = defaultdict(list)
costs = defaultdict(list)
last_key_spend = None
for line in open("results/llm_calls.jsonl"):
    d = json.loads(line)
    if not d["dry_run"]:
        times[d["alias_requested"]].append(d["elapsed_seconds"])
        ct = d.get("usage", {}).get("completion_tokens")
        if ct is not None:
            tokens[d["alias_requested"]].append(ct)
        c = d.get("cost_usd")
        if c is not None:
            costs[d["alias_requested"]].append(c)
        if d.get("key_spend_total_usd") is not None:
            last_key_spend = d["key_spend_total_usd"]

stats = defaultdict(lambda: {"total": 0, "pass": 0, "mismatch": 0, "build_fail": 0, "no_output": 0, "api_error": 0})
for r in rows:
    s = stats[r["alias"]]
    s["total"] += 1
    s[r["result"]] += 1

print(f"{'alias':<28} {'total':>5} {'pass':>5} {'mismatch':>9} {'build_fail':>11} {'no_output':>10} {'api_error':>10} {'avg_secs':>9} {'avg_tokens':>10} {'total_cost':>11}")
grand_total = 0
for alias, s in stats.items():
    avg = sum(times[alias]) / len(times[alias]) if times[alias] else 0
    avg_tok = sum(tokens[alias]) / len(tokens[alias]) if tokens[alias] else 0
    alias_cost = sum(costs[alias])
    grand_total += alias_cost
    print(f"{alias:<28} {s['total']:>5} {s['pass']:>5} {s['mismatch']:>9} {s['build_fail']:>11} {s['no_output']:>10} {s['api_error']:>10} {avg:>9.1f} {avg_tok:>10.0f} ${alias_cost:>10.4f}")

print(f"\ntotal cost this run (summed from per-call cost_usd): ${grand_total:.4f}")
if last_key_spend is not None:
    print(f"account total spend so far (from gateway, cumulative): ${last_key_spend:.4f}")