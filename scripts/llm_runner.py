#!/usr/bin/env python3
# Shared "run the just-built binary + grade its output" helpers.
#
# One grading truth for both the Python values-feedback loop (generate_with_repair.py
# --values-check) and the bash run scripts, so the run-binaries diff matches the
# analysis tolerance (funnel.py uses TOL=0.011, not exact text equality).
#
# bash usage (grade an already-captured output file against native):
#   python3 scripts/llm_runner.py --grade --native <native.txt> --out <out.txt>
# prints one line:  pass | mismatch | no_output, plus " <right>/<total>"
# Optional --first-k N to also print the mismatch summary (for logs/debug).
#
# python usage: import run_binary, grade_outputs, summarize_mismatch

import argparse
import os
import re
import subprocess
import sys

TOL = 0.011
FIRST_K_DEFAULT = 10

DUMP_BLOCK_RE = re.compile(r"begin dump:[^\n]*\n(.*?)end\s+dump", re.DOTALL)
NUM_RE = re.compile(r"-?\d+\.\d+")


def is_dump_output(path):
    """True if a captured output file actually contains a polybench dump block.

    Guards against counting QEMU/FreeDOS boot artifacts (empty, 0 B, or a small non-dump
    binary blob like a boot sector) as a real graded run — the 8086 'mismatch'-bucket bug.
    Inline-asm kernels that mutate memory but never print DUMP_ARRAYS come back here as
    no_output (the x86 'compiles, runs, stores nothing' class), not mismatch."""
    if not path or not os.path.exists(path) or os.path.getsize(path) == 0:
        return False
    text = open(path, errors="replace").read()
    return bool(DUMP_BLOCK_RE.search(text))


def values(path):
    """Extract the numbers printed between begin/end dump markers."""
    if not os.path.exists(path):
        return None
    text = open(path, errors="replace").read()
    blocks = DUMP_BLOCK_RE.findall(text)
    if not blocks:
        return None
    return [float(x) for b in blocks for x in NUM_RE.findall(b)]


def _sandbox_preexec(cpu_s, mem_kb, out_kb):
    """preexec_fn for subprocess.Popen: apply the run sandbox in the CHILD only.

    Mirrors run_x86.sh's `ulimit -t 30 -v 4000000 -f 1000` in the spawned binary:
      cpu_s   - CPU seconds (RLIMIT_CPU)
      mem_kb  - virtual address space in KiB (RLIMIT_AS; bash -v is in KiB)
      out_kb  - max output file size in KiB (RLIMIT_FSIZE)
    The first arg (unlimited) is the hard limit, which must be set BEFORE dropping the
    soft limit on 'v'/'f' so a non-root process may lower it (mirrors the bash parent).
    Runs as the real child, so it never caps the long-lived Python orchestrator."""
    import resource
    r_soft = {"t": cpu_s, "v": mem_kb * 1024, "f": out_kb * 1024}
    w = {"t": resource.RLIMIT_CPU, "v": resource.RLIMIT_AS, "f": resource.RLIMIT_FSIZE}
    for key in ("v", "f"):
        try:
            resource.setrlimit(w[key], (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
        except (ValueError, OSError):
            pass  # best effort — don't fail the run because we couldn't raise the hard limit
    for key, lim in w.items():
        try:
            resource.setrlimit(lim, (r_soft[key], r_soft[key]))
        except (ValueError, OSError):
            pass  # best effort — don't fail the run because we couldn't sandbox


def run_binary(exe, out_path, err_log, timeout=15, cpu=30, mem_mb=4000, out_kb=1000):
    """Run a native binary under the same sandbox bash uses (run_x86.sh), capturing
    stdout to out_path. Returns (returncode, timed_out). Mirrors:
        ( ulimit -t 30 -v 4000000 -f 1000; timeout 15s "$exe" ... )

    The limits are applied ONLY to the child (preexec_fn), so the long-lived Python
    orchestrator keeps its own (unlimited) address space — the old code wrongly set
    RLIMIT_AS on the Python process in BYTES (mem_mb*1024 ≈ 4MB, off by ~1000x from
    bash's KiB), which made every subsequent allocation in the run fail with
    MemoryError (seen on 2026-09-29 in subprocess.run's timeout-read and append_call).
    """
    if os.path.exists(out_path):
        os.remove(out_path)
    try:
        import resource  # noqa: F401  (probe once; preexec_fn runs in the child)
    except ImportError:
        # non-POSIX without resource: no sandbox possible, run unsandboxed
        try:
            with open(out_path, "w") as outf, open(err_log, "a") as errf:
                proc = subprocess.run([exe], stdin=subprocess.DEVNULL, stdout=outf,
                                      stderr=errf, timeout=timeout)
            return proc.returncode, False
        except subprocess.TimeoutExpired:
            with open(err_log, "a") as errf:
                errf.write("\n[run timed out after %ss]\n" % timeout)
            return 124, True
    preexec = _sandbox_preexec(cpu, mem_mb * 1024, out_kb * 1024)
    try:
        with open(out_path, "w") as outf, open(err_log, "a") as errf:
            proc = subprocess.run([exe], stdin=subprocess.DEVNULL, stdout=outf,
                                  stderr=errf, timeout=timeout, preexec_fn=preexec)
        return proc.returncode, False
    except subprocess.TimeoutExpired:
        with open(err_log, "a") as errf:
            errf.write("\n[run timed out after %ss]\n" % timeout)
        return 124, True


def grade_outputs(native_path, out_path):
    """Compare two dump files. Returns dict:
        {result, right, total, max_abs_diff, first_k:[(i, native, got)]}
    result: pass | mismatch | no_output | empty | unparseable
    Uses the same TOL as the analysis grader."""
    if not os.path.exists(out_path):
        return {"result": "no_output", "right": 0, "total": 0,
                "max_abs_diff": None, "first_k": []}
    if os.path.getsize(out_path) == 0:
        return {"result": "empty", "right": 0, "total": 0,
                "max_abs_diff": None, "first_k": []}
    native, out = values(native_path), values(out_path)
    if native is None or out is None or not native:
        return {"result": "unparseable", "right": 0, "total": 0,
                "max_abs_diff": None, "first_k": []}
    total = len(native)
    if len(out) != total:
        return {"result": "mismatch", "right": 0, "total": total,
                "max_abs_diff": None,
                "first_k": [{"i": 0, "native": native[0] if native else None,
                             "got": out[0] if out else None}],
                "len_native": total, "len_got": len(out)}
    right = 0
    max_diff = 0.0
    diffs = []
    for i, (a, b) in enumerate(zip(native, out)):
        d = abs(a - b)
        if d <= TOL:
            right += 1
        else:
            diffs.append((i, a, b))
            max_diff = max(max_diff, d)
    result = "pass" if right == total else "mismatch"
    return {"result": result, "right": right, "total": total,
            "max_abs_diff": max_diff, "first_k": diffs[:FIRST_K_DEFAULT],
            "len_native": total, "len_got": len(out)}


def summarize_mismatch(g, first_k=None):
    """Turn a grade_outputs dict into a compact, token-cheap text description for the
    values-repair prompt. Emphasizes sign/scale/magnitude bugs (the observed 8086
    failure class: 32-bit int math in 64-bit registers producing huge integers)."""
    k = first_k if first_k is not None else FIRST_K_DEFAULT
    lines = []
    if g.get("result") == "no_output":
        return "The program produced no output file at all."
    if g.get("result") == "empty":
        return "The program produced an empty output (no values were printed)."
    if g.get("result") == "unparseable":
        return "The output could not be parsed as numbers."
    if g.get("result") == "timeout":
        return ("The program ran for too long and was killed (likely an infinite or "
                "pathologically slow loop). Check that every loop counter actually "
                "advances and terminates.")
    if g.get("result") == "run_fail":
        rc = g.get("rc")
        return (f"The program crashed at runtime (exit code {rc}). Likely an illegal "
                "instruction, an invalid memory access (reading/writing past an array, "
                "a wrong base/index register, a clobbered pointer) or a stack problem.")
    if g.get("len_native") != g.get("len_got"):
        return (f"Wrong output length: native={g.get('len_native')}, got={g.get('len_got')} "
                f"(expected {g.get('total')} values).")
    lines.append(f"Output mismatch: {g.get('right')}/{g.get('total')} values correct.")
    m = g.get("max_abs_diff")
    if m is not None and m >= 1:
        lines.append(f"Largest absolute error magnitude: {m:.3g} (suggests "
                     f"overflow/width/indexing, not floating-point rounding).")
    pairs = (g.get("first_k") or [])[:k]
    if pairs:
        lines.append(f"First {len(pairs)} mismatching value(s):")
        for it in pairs:
            if isinstance(it, dict):
                lines.append(f"  index {it['i']}: native={it['native']:g} got={it['got']:g}")
            else:                       # tuples from grade_outputs
                i, a, b = it
                lines.append(f"  index {i}: native={a:g} got={b:g}")
    return "\n".join(lines)


def main():
    p = argparse.ArgumentParser(description="grade a model binary output vs native")
    p.add_argument("--grade", action="store_true")
    p.add_argument("--native", required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--first-k", type=int, default=0)
    p.add_argument("--dump-only", action="store_true",
                   help="require a real DUMP_ARRAYS block in --out; boot artifacts / "
                        "non-dump binaries grade as no_output instead of mismatch")
    args = p.parse_args()
    if args.dump_only and not is_dump_output(args.out):
        # no real dump block -> no_output (cache/empty/blob), not a graded mismatch
        print("no_output 0/0")
        return
    g = grade_outputs(args.native, args.out)
    line = f"{g['result']} {g.get('right',0)}/{g.get('total',0)}"
    if args.first_k and g["result"] == "mismatch":
        line += "\n" + summarize_mismatch(g)
    print(line)


if __name__ == "__main__":
    main()
