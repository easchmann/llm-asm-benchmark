#!/usr/bin/env python3
# check the harness before spending API money: hand-written solutions pass,
# mutants fail, and the extractor handles typical reply shapes
# usage: python3 scripts/validate_desc.py

import glob
import json
import os
import re
import sys
import tempfile

from gen_desc import DESC, V2, check, crash_site, extract_asm, fail_lines, feedback_v2, plan_has_code

ASM = ".globl axpy\naxpy:\n    ret\n"
REPLIES = [
    ("fenced", f"```asm\n{ASM}```", "fenced"),
    ("prose around fence", f"Here is the code:\n```asm\n{ASM}```\nDone.", "fenced"),
    ("think then fence", f"<think>draft\n```asm\n{ASM}```\n</think>\n```s\n{ASM}```", "fenced"),
    ("leaked opening think tag", f"<seed:think>reasoning...\n```asm\n{ASM}```", "fenced"),
    ("two blocks", f"```asm\n{ASM}```\nFixed:\n```asm\n{ASM}```", "last_of_2_blocks"),
    ("unfenced", ASM, "unfenced"),
    ("no globl", "```asm\nfoo:\n    ret\n```", "no_globl"),
]
PLANS = [
    ("prose plan", "- n arrives in %rdi, a in %xmm0.\n- Loop i from 0 to n-1, address 8*i(%rsi).\nNo callee-saved registers.", False),
    ("register table", "```\nrdi  n\nrsi  x\nxmm0 a\n```\nLoop over i.", False),
    ("fenced code", "Plan:\n```asm\n    movsd (%rsi,%rcx,8), %xmm1\n```", True),
    ("unfenced code", "    addq $1, %rcx\n" * 5, True),
]


def report(ok, label, detail):
    print(f"{'ok ' if ok else 'BAD'} {label}: {detail}")
    return ok


def normalize(text):
    return re.sub(r"\s+", " ", text).replace("( ", "(").replace(" )", ")").strip()


def signature_matches(kdir):
    spec = json.load(open(os.path.join(kdir, "spec.json")))
    driver = normalize(open(os.path.join(kdir, "driver.c")).read())
    return normalize(spec["signature"]) + ";" in driver


def main():
    work = tempfile.mkdtemp(prefix="desc_validate_")
    ok = True

    for spec in sorted(glob.glob(os.path.join(DESC, "*", "spec.json"))):
        kdir = os.path.dirname(spec)
        kernel = os.path.basename(kdir)
        missing = [f for f in ("driver.c", "ref.c", "L0.txt", "L1.txt", "L2.txt", "L3.txt")
                   if not os.path.exists(os.path.join(kdir, f))]
        if missing:
            ok &= report(False, f"{kernel} files", "missing " + ", ".join(missing))
            continue
        ok &= report(signature_matches(kdir), f"{kernel} signature", "spec.json matches driver.c")

    for hand in sorted(glob.glob(os.path.join(DESC, "*", "hand.S"))):
        kdir = os.path.dirname(hand)
        kernel = os.path.basename(kdir)
        stage, sizes, _, _ = check(hand, kdir, os.path.join(work, kernel))
        ok &= report(stage == "pass", f"{kernel} hand.S", f"{stage} {sizes}")
        diag = check(hand, kdir, os.path.join(work, kernel + "_diag"), diag=True)[0]
        ok &= report(diag == "pass", f"{kernel} hand.S ({V2})", diag)
        for mutant in sorted(glob.glob(os.path.join(kdir, "mutants", "*.S"))):
            name = os.path.basename(mutant)[:-2]
            stage, sizes, details, _ = check(mutant, kdir, os.path.join(work, f"{kernel}_{name}"))
            first = next((l for l in details.splitlines() if ": fail" in l), "")
            ok &= report(stage != "pass", f"{kernel} mutant {name}", f"{stage} {first}")
            diag = check(mutant, kdir, os.path.join(work, f"{kernel}_{name}_diag"), diag=True)[0]
            ok &= report(diag == stage, f"{kernel} mutant {name} ({V2})", diag)

    unfinished = os.path.join(work, "unfinished.S")
    open(unfinished, "w").write(".globl axpy\naxpy:\n    movq %rdi, %rax\n")
    stage = check(unfinished, os.path.join(DESC, "axpy"), os.path.join(work, "unfinished"))[0]
    ok &= report(stage == "incomplete", "answer without ret", stage)

    axpy = os.path.join(DESC, "axpy")
    hand = open(os.path.join(axpy, "hand.S")).read().replace("\naxpy:\n", "\naxpy:\n    xorl %eax, %eax\n    movq 8(%rax), %rax\n", 1)
    line = hand.splitlines().index("    movq 8(%rax), %rax") + 1
    crashing = os.path.join(work, "crashing.S")
    open(crashing, "w").write(hand)
    stage, _, details, _ = check(crashing, axpy, os.path.join(work, "crashing"), diag=True)
    where, text = crash_site(details, crashing, axpy, os.path.join(work, "crashing"))
    ok &= report(stage == "crash" and where == f"line {line}" and "address was 0x8." in text, "crash location", where)

    gemm = os.path.join(DESC, "gemm")
    details = check(os.path.join(gemm, "mutants", "skip_beta_scaling.S"), gemm, os.path.join(work, "gemm_wrong"), diag=True)[2]
    ok &= report("first wrong element C(" in details, "first wrong element", next(iter(fail_lines(details)), ""))

    broken = os.path.join(work, "broken.S")
    open(broken, "w").write(".globl axpy\naxpy:\n    movq %rdi, %rax, %rbx\n    ret\n")
    details = check(broken, axpy, os.path.join(work, "broken"), diag=True)[2]
    note = feedback_v2("assemble_fail", details, open(broken).read(), "", [])
    ok &= report("    3 | movq %rdi, %rax, %rbx" in note, "assembler error with source line", note.splitlines()[1])

    for label, reply, want in REPLIES:
        _, how = extract_asm(reply, "axpy")
        ok &= report(how == want, f"extract {label}", how)

    for label, plan, want in PLANS:
        ok &= report(plan_has_code(plan) == want, f"plan_has_code {label}", plan_has_code(plan))

    print("\nall good" if ok else "\nproblems found")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
