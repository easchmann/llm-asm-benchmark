#!/usr/bin/env python3
# rule-based pass: X from the extraction stage, A/B from assembler/linker messages, C from the ABI checks and
# crash sites, D for disallowed ISA and misaligned aligned accesses, E from the size sweep; the rest is "pending"
# usage: python3 scripts/ea_rules.py   (needs scripts/ea_retest.py results)

import collections
import csv
import os
import re
import subprocess

from ea_common import OUT, attempts, load_retest

FIELDS = ["run", "model", "kernel", "kernel_category", "level", "repeat", "attempt", "stage",
          "category", "subtype", "secondary", "evidence", "source", "tag"]
AS_ERR_RE = re.compile(r"(?:\.S:|line )(\d+): Error: (.*)")
INTEL_RE = re.compile(r"\[|\bptr\b|\.intel_syntax|(^|[\s,])(r[a-d]x|r[sb]p|r[sd]i|r\d+[dwb]?|e[a-d]x|e[sb]p|e[sd]i|[a-d]l|[xy]mm\d+)\b")
ABI_RE = re.compile(r"callee-saved register (\w+) not preserved|stack pointer not restored")
INSN_RE = re.compile(r"\(instruction: (.*)\)$", re.M)
ALIGNED_RE = re.compile(r"^v?(movap[sd]|movdqa\w*|movntp[sd]|movntdq\w*)$")
UNALIGNED = re.compile(r"^(push\w*|pop\w*|prefetch\w*|pinsr\w|pextr\w|pmov[sz]x\w*|pbroadcast\w*|movu[ps][sd]|movdqu|lddqu|movs[sd]|movq|movd|movhp[sd]|movlp[sd]|cvt\w+|\w+s[sd])$")


def symbol_error(msg):
    if re.search(r"is already defined", msg):
        return "duplicate_symbol"
    if re.search(r"local label .* is not defined|\.size expression .* constant|symbol .* undefined|undefined", msg):
        return "undefined_symbol"
    return None


def assembler(a, log, lines):
    errs = AS_ERR_RE.findall(log) or AS_ERR_RE.findall("\n".join(a["rec"].get("error_lines") or []))
    errs = [(int(n), msg) for n, msg in errs]
    intel_file = ".intel_syntax" in "\n".join(lines)
    kinds = []
    for n, msg in errs:
        src = lines[n - 1].split("#")[0].strip() if 0 < n <= len(lines) else ""
        intel = intel_file or ("%" not in src and INTEL_RE.search(src))
        if intel and not re.search(r"pseudo-op|directive", msg):
            kinds.append("intel_syntax")
        elif "no such instruction" in msg:
            kinds.append("unknown_mnemonic")
        elif "suffix" in msg or "ambiguous operand size" in msg:
            kinds.append("size_suffix")
        elif src.startswith(".") or re.search(r"pseudo-op|directive", msg):
            kinds.append("directive")
        elif re.search(r"operand|register|unsupported instruction|scale factor|base/index|memory reference|expression|displacement", msg):
            ops = src.split(None, 1)[1] if " " in src else ""
            kinds.append("operand_order" if re.search(r",\s*\$[^,]*$", ops) else "invalid_operand")
        else:
            kinds.append("other_syntax")
    cpp = re.search(r":\d+(?::\d+)?: error: (.*)", log) if not errs else None
    if cpp:
        plain = not subprocess.run(["as", "--64", "-o", "/dev/null", a["asm"]], capture_output=True).returncode
        return "A", "preprocessor", (f"C preprocessor error on a # comment line: {cpp.group(1)}; "
                                     + ("assembles without cpp" if plain else "does not assemble without cpp either"))
    if not errs:
        errs = [(0, m) for m in re.findall(r"Error: (.*)", log)] or [(0, "no message")]
    n, msg = errs[0]
    if symbol_error(msg):
        return "B", symbol_error(msg), f"assembler: {msg}"
    count = ", ".join(f"{k} x{v}" for k, v in collections.Counter(kinds).most_common())
    src = lines[n - 1].strip() if 0 < n <= len(lines) else ""
    return "A", kinds[0] if kinds else "other_syntax", f"{len(errs)} errors ({count}); first: line {n}: {msg} | {src}"


def linker(a, log):
    m = re.search(r"undefined reference to `([^']+)'", log)
    if m:
        return "B", "undefined_symbol", m.group(0)
    for pat, sub in [(r"relocation R_X86_64_\w+ against .*?(-fPIE|PIE)", "pie_relocation"),
                     (r"multiple definition of `[^']+'", "duplicate_symbol")]:
        m = re.search(pat, log)
        if m:
            return "B", sub, m.group(0)[:200]
    err = next((l for l in log.splitlines() if re.search(r"error|Error", l) and "warning" not in l), "")
    return "B", "other_link", err[:200]


def abi(a, rt):
    out = os.path.join(a["run_dir"], "code", a["tag"] + ".out")
    text = open(out, errors="replace").read() if os.path.exists(out) else ""
    text += "\n".join(rt.get("orig", {}).get("fails", []))
    m = ABI_RE.search(text)
    if not m:
        return None
    regs = sorted(set(ABI_RE.findall(text)) - {""})
    sub = "callee_saved" if m.group(1) else "stack_pointer"
    return "C", sub, ("callee-saved not preserved: " + ", ".join(regs) if regs else m.group(0))


def crash(a, rt, lines):
    o = rt.get("orig", {})
    if o.get("stage") != "crash" or a["stage"] != "crash":
        return None
    where, text, fault = o.get("crash_where"), o.get("crash_text", ""), o.get("fault") or {}
    if where == "invalid address":
        return "C", "stack_pointer", text.splitlines()[0][:200]
    m = INSN_RE.search(text)
    insn = m.group(1).strip() if m else ""
    mnem, _, ops = insn.partition(" ")
    if fault.get("signal") == "04" and re.search(r"zmm|%k[0-7]|\{", insn):
        return "D", "isa", f"SIGILL at {insn}"
    legacy_packed = not mnem.startswith("v") and re.search(r"(p[sd]|^p\w+)$", mnem) and not UNALIGNED.match(mnem)
    if fault.get("addr") == "0x0000000000000000" and "(" in ops and (ALIGNED_RE.match(mnem) or legacy_packed):
        frame = "%rsp" in ops or ("%rbp" in ops and re.search(r"mov\w*\s+%rsp,\s*%rbp", "\n".join(lines)))
        return ("C", "stack_alignment", f"{where}: {insn}") if frame else ("D", "aligned_access", f"{where}: {insn}")
    return None


def dims(label):
    return [int(x) for x in re.findall(r"(?:^|[x_n])(\d+)", label.split("_n")[-1] if "_n" in label else label)]


def sweep(rt):
    mult, tail = rt.get("mult"), rt.get("tail")
    if not mult or not tail or mult["stage"] != "pass" or tail["stage"] == "pass":
        return None
    failed = [s for s, v in tail["sizes"].items() if v == "fail"]
    if tail["stage"] != "wrong":
        nxt = (tail.get("fault") or {}).get("size")
        failed += [nxt] if nxt else []
    small = failed and all(max(dims(s)) < 8 for s in failed)
    sub = "small_n" if small else "tail"
    return "E", sub, f"sweep: all multiple-of-8 sizes pass; tail sizes {tail['stage']}, failing: {', '.join(failed) or 'n/a'}"


def classify(a):
    rt = load_retest(a)
    lines = open(a["asm"], errors="replace").read().splitlines() if os.path.exists(a["asm"]) else []
    if a["stage"] in ("extract_fail", "incomplete"):
        return ("X", "no_code" if a["stage"] == "extract_fail" else "incomplete", a["stage"]), ""
    if a["stage"] == "assemble_fail":
        return assembler(a, rt.get("build", ""), lines), ""
    if a["stage"] == "link_fail":
        return linker(a, rt.get("build", "")), ""
    e = sweep(rt)
    c = abi(a, rt) or crash(a, rt, lines)
    if c:
        return c, ("E" if e else "")
    return e, ""


def main():
    rows = []
    for a in attempts():
        res, secondary = classify(a)
        cat, sub, ev = res if res else ("", "", "")
        rows.append({**{k: a[k] for k in FIELDS if k in a}, "category": cat, "subtype": sub, "secondary": secondary,
                     "evidence": ev, "source": "rule" if cat else "pending"})
    with open(os.path.join(OUT, "rules.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, FIELDS)
        w.writeheader()
        w.writerows(rows)
    c = collections.Counter((r["run"], r["category"] or "pending") for r in rows)
    for run in sorted({r["run"] for r in rows}):
        print(run, {k[1]: v for k, v in sorted(c.items()) if k[0] == run})
    print("pending:", sum(r["source"] == "pending" for r in rows), "of", len(rows))


if __name__ == "__main__":
    main()
