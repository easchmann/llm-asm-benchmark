#!/usr/bin/env python3
# check assembler/compiler/linker errors in each model's compile log
# usage: python3 analyze_errors.py [run_dir|code_dir]
#   (default: .work/llm_compare; a run dir's code/ is used when it has no .compile_err.log directly)

import glob, os, re, sys
from collections import Counter, defaultdict

work = sys.argv[1] if len(sys.argv) > 1 else ".work/llm_compare"
if not glob.glob(os.path.join(work, "*.compile_err.log")) and os.path.isdir(os.path.join(work, "code")):
    work = os.path.join(work, "code")

def messages(path):
    msgs, symbols = [], []
    for line in open(path, errors="replace"):
        if line.startswith("collect2"):
            continue # just the "ld returned 1" summary
        if "undefined reference to" in line:
            m = re.search(r"undefined reference to [`'](.+?)'", line)
            msgs.append("undefined reference to `X'")
            if m:
                symbols.append(m.group(1))
        elif "Error:" in line or " error:" in line:
            msg = re.split(r"Error: | error: ", line, maxsplit=1)[-1].strip()
            # group "for `mov'" / "for `xor'" etc. together
            msgs.append(re.sub(r"[`'][^`']*'", "`X'", msg))
    return msgs, symbols

per_model = {}
all_symbols = Counter()
for path in sorted(glob.glob(os.path.join(work, "*.compile_err.log"))):
    base = os.path.basename(path)[:-len(".compile_err.log")]
    kernel, slug = base.split("__", 1)
    model = slug.replace("_", "/", 1)
    msgs, symbols = messages(path)
    per_model[(kernel, model)] = Counter(msgs)
    all_symbols.update(symbols)

print("== per model ==")
for (kernel, model), c in per_model.items():
    total = sum(c.values())
    if total == 0:
        print(f"{kernel:<6} {model:<28} no errors (compiled)")
        continue
    print(f"{kernel:<6} {model:<28} {total} errors")
    for msg, n in c.most_common(3):
        print(f"           {n:>3}x  {msg[:90]}")

by_msg = defaultdict(lambda: [0, set()])
for (kernel, model), c in per_model.items():
    for msg, n in c.items():
        by_msg[msg][0] += n
        by_msg[msg][1].add(model)

print("\n== error types across models ==")
print(f"{'models':>6} {'total':>6}  message")
for msg, (n, models) in sorted(by_msg.items(), key=lambda kv: (-len(kv[1][1]), -kv[1][0])):
    print(f"{len(models):>6} {n:>6}  {msg[:90]}")

if all_symbols:
    print("\nundefined symbols the models referenced from asm:")
    for sym, n in all_symbols.most_common():
        print(f"  {n:>3}x  {sym}")