#!/usr/bin/env python3
# for every model, how far did each run get?
#   no_answer -> format -> c_syntax -> operands -> assembler -> link -> runtime -> wrong_output -> pass
# Generates @code in `python3 scripts/funnel.py` ... `--` ... a run's own dir:
#   python3 scripts/funnel.py 8086=results/llm_compare_specific_20260928_1600:8086 \
#                             x86=results/x86_20260928_1700:x86
# (generated code/snapshots are found in <run>/code/; `label=run:work:suffix` still works)

import csv, json, os, re, sys
from collections import Counter, defaultdict

TOL = 0.011 # values print with 2 decimals
ORDER = ["no_answer", "format", "c_syntax", "operands", "assembler", "link",
         "build_other", "runtime", "wrong_output", "pass_suspect", "pass"]

# error messages -> stage, checked in pipeline order (an early stage hides the later ones)
PATTERNS = [
    ("c_syntax", r"missing terminating|expected string literal|expected .\(. before|expected expression|expected statement|too many decimal points"),
    ("operands", r"impossible constraint|not enough registers|constraint lacks|read-only location|can't find a register|matching constraint|undefined named operand"),
    ("assembler", r"Error:"),                       # gas
    ("link", r"undefined reference"),
]
FORMAT_MSGS = r"stray|invalid preprocessing directive|invalid suffix|unknown type name|expected declaration or statement at end of input|expected identifier or .\(. before .%"


def region(path):
    # line-anchored markers, so a prose line that merely mentions "#pragma endscop" doesn't end the region
    if not os.path.exists(path):
        return None
    src = open(path, errors="replace").read()
    m = re.search(r"(?m)^#pragma scop[^\n]*\n(.*?)\n#pragma endscop", src, re.DOTALL)
    return m.group(1) if m else None


def format_problem(reg):
    if "```" in reg:
        return "markdown fence in the code"
    if re.search(r"</?(?:\w+:)?think(?:ing)?>", reg):
        return "thinking text in the code"
    if "asm" not in reg.lower():
        return "no asm() block"
    return None


def values(path):
    text = open(path, errors="replace").read()
    blocks = re.findall(r"begin dump:[^\n]*\n(.*?)end\s+dump", text, re.DOTALL)
    return [float(x) for b in blocks for x in re.findall(r"-?\d+\.\d+", b)] if blocks else None


def grade_output(out_path, native_path):
    if not os.path.exists(out_path):
        return "no output file"
    if os.path.getsize(out_path) == 0:
        return "empty"
    out, native = values(out_path), values(native_path) if os.path.exists(native_path) else None
    if out is None or native is None:
        return "unparseable"
    if len(out) != len(native):
        return "wrong length"
    n = sum(abs(a - b) <= TOL for a, b in zip(native, out))
    if all(v == 0 for v in out):
        return "all zeros"
    return "partial (%d/%d right)" % (n, len(native)) if n else "wrong values"


def api_detail(calls, kernel, alias):
    for d in reversed(calls):
        if d.get("kernel") == kernel and d.get("alias_requested") == alias:
            return str(d.get("underlying_model", "")).replace("error: ", "")[:70]
    return "no log entry"


def classify(row, results, work, suffix, calls):
    kernel, alias, result = row["kernel"], row["alias"], row["result"]
    slug = alias.replace("/", "_")
    reg = region(os.path.join(work, f"{kernel}__{slug}.c"))

    if result == "api_error":
        return "no_answer", api_detail(calls, kernel, alias)
    if result == "build_fail":
        if reg is not None and format_problem(reg):
            return "format", format_problem(reg)
        log = os.path.join(work, f"{kernel}__{slug}.compile_err.log")
        lines = [l.strip() for l in open(log, errors="replace")] if os.path.exists(log) else []
        errs = [l for l in lines if "Error:" in l or " error:" in l or "undefined reference" in l]
        errs = [l for l in errs if not l.startswith("collect2")]
        for l in errs: # prose that leaked into the source
            if re.search(FORMAT_MSGS, l):
                return "format", re.split(r"Error: | error: ", l, maxsplit=1)[-1][:70]
        for stage, pat in PATTERNS:
            for l in errs:
                if re.search(pat, l):
                    return stage, re.split(r"Error: | error: ", l, maxsplit=1)[-1][:70]
        return "build_other", (errs[0][:70] if errs else "no compile log found")
    if result in ("no_output", "run_fail", "timeout"):
        return "runtime", result
    if result == "mismatch":
        out = os.path.join(results, f"{kernel}__{slug}_{suffix}.txt")
        return "wrong_output", grade_output(out, os.path.join(results, f"{kernel}_native.txt"))
    if result == "pass":
        if reg is not None and (format_problem(reg) or re.search(r"(?m)^\s*(for|while)\s*\(", reg)):
            return "pass_suspect", "passes, but no asm block or C loops left in place"
        return "pass", "asm block present"
    return "build_other", f"unknown result '{result}'"


runs = []
for arg in sys.argv[1:]:
    label, spec = arg.split("=", 1)
    parts = spec.split(":")
    if len(parts) == 2:
        results, suffix = parts
        work = os.path.join(results, "code")   # run dir layout: code lives in results/<run>/code/
    elif len(parts) == 3:
        results, work, suffix = parts          # legacy: results:work:suffix
    else:
        sys.exit(f"bad spec '{spec}' (want label=run[:work]:suffix)")
    csv_path = os.path.join(results, "comparison.csv")
    if not os.path.exists(csv_path):
        sys.exit(f"{csv_path} not found (looking from {os.getcwd()}) -- run from the repo root")
    calls_path = os.path.join(results, "llm_calls.jsonl")
    calls = [json.loads(l) for l in open(calls_path)] if os.path.exists(calls_path) else []
    runs.append((label, results, work, suffix, list(csv.DictReader(open(csv_path))), calls))
if not runs:
    sys.exit("usage: python3 funnel.py label=results_run_dir[:work_dir]:suffix [label=...]\n"
             "  e.g. python3 funnel.py 8086=results/llm_compare_20260928_1600:8086\n"
             "            x86=results/x86_20260928_1700:x86")

table = defaultdict(dict)  # (kernel, alias) -> label -> (stage, detail)
counts = {label: Counter() for label, *_ in runs}
for label, results, work, suffix, rows, calls in runs:
    for row in rows:
        stage, detail = classify(row, results, work, suffix, calls)
        table[(row["kernel"], row["alias"])][label] = (stage, detail)
        counts[label][stage] += 1

labels = [r[0] for r in runs]
w = max(12, max(len(s) for s in ORDER) + 1)
print(f"{'kernel':<7} {'alias':<27}" + "".join(f"{l:<{w}}" for l in labels))
for (kernel, alias), cells in sorted(table.items()):
    print(f"{kernel:<7} {alias:<27}" + "".join(f"{cells.get(l, ('-', ''))[0]:<{w}}" for l in labels))

print("\n== details ==")
for (kernel, alias), cells in sorted(table.items()):
    for l in labels:
        if l in cells:
            print(f"{kernel:<7} {alias:<27} [{l}] {cells[l][0]}: {cells[l][1]}")

print("\n== how many runs ended at each stage ==")
print(f"{'stage':<14}" + "".join(f"{l:>{w}}" for l in labels))
for stage in ORDER:
    if any(counts[l][stage] for l in labels):
        print(f"{stage:<14}" + "".join(f"{counts[l][stage]:>{w}}" for l in labels))