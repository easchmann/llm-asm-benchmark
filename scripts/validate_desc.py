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

from gen_desc import DESC, check, extract_asm

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
        stage, sizes, _ = check(hand, kdir, os.path.join(work, kernel))
        ok &= report(stage == "pass", f"{kernel} hand.S", f"{stage} {sizes}")
        for mutant in sorted(glob.glob(os.path.join(kdir, "mutants", "*.S"))):
            name = os.path.basename(mutant)[:-2]
            stage, sizes, details = check(mutant, kdir, os.path.join(work, f"{kernel}_{name}"))
            first = next((l for l in details.splitlines() if ": fail" in l), "")
            ok &= report(stage != "pass", f"{kernel} mutant {name}", f"{stage} {first}")

    for label, reply, want in REPLIES:
        _, how = extract_asm(reply, "axpy")
        ok &= report(how == want, f"extract {label}", how)

    print("\nall good" if ok else "\nproblems found")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
