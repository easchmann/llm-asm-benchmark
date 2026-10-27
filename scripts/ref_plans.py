#!/usr/bin/env python3
# reference plans for --plan-from: export the plan prompt of every kernel and level, check a plan directory
# usage: python3 scripts/ref_plans.py export plans/claude-opus-5-5   writes <kernel>/<level>.prompt.txt
#        python3 scripts/ref_plans.py check plans/claude-opus-5-5    every <kernel>/<level>.txt present, prompt current,
#        no code in the plan, and the implementation of the plan <kernel>/<level>.S passes the driver

import glob
import os
import sys
import tempfile

from gen_desc import DESC, PLAN, build_prompt, check, load_kernel, plan_has_code
from md import heading, table

LEVELS = ["L0", "L1", "L2", "L3"]


def tasks():
    for spec in sorted(glob.glob(os.path.join(DESC, "*", "spec.json"))):
        kernel = os.path.basename(os.path.dirname(spec))
        for level in LEVELS:
            _, spec_json, description = load_kernel(kernel, level)
            yield kernel, level, build_prompt(spec_json, description, PLAN)


def main():
    action, plan_dir = sys.argv[1:3]
    rows, ok = [], True
    work = tempfile.mkdtemp(prefix="ref_plans_")
    for kernel, level, prompt in tasks():
        base = os.path.join(plan_dir, kernel, level)
        if action == "export":
            os.makedirs(os.path.dirname(base), exist_ok=True)
            with open(base + ".prompt.txt", "w") as f:
                f.write(prompt)
            continue
        plan = open(base + ".txt").read().strip() if os.path.exists(base + ".txt") else ""
        stale = not os.path.exists(base + ".prompt.txt") or open(base + ".prompt.txt").read() != prompt
        asm = base + ".S"
        stage = check(asm, os.path.join(DESC, kernel), os.path.join(work, f"{kernel}_{level}"))[0] if os.path.exists(asm) else "no code"
        problems = [p for p, bad in (("missing", not plan), ("prompt changed", stale),
                                     ("has code", plan and plan_has_code(plan)), (stage, stage != "pass")) if bad]
        ok &= not problems
        rows.append([kernel, level, len(plan), stage, ", ".join(problems) or "ok"])
    if action == "export":
        print(f"wrote {len(LEVELS) * len(glob.glob(os.path.join(DESC, '*', 'spec.json')))} plan prompts to {plan_dir}")
        return
    heading(f"Reference plans in {plan_dir}")
    table(["kernel", "level", "chars", "implementation", "status"], rows)
    print("all plans present and verified" if ok else "problems found")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
