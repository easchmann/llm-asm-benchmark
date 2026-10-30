#!/usr/bin/env python3
# re-test saved failed assemblies (no model calls): full assembler/linker output, diagnostic build at the
# original sizes, and two size sweeps: every vectorisable dimension a multiple of 8 ("mult") or not ("tail")
# usage: python3 scripts/ea_retest.py [--jobs 24] [--check-drivers]
# resumable: attempts with a result file are skipped

import argparse
import json
import os
import re
import shutil
import tempfile
from multiprocessing import Pool

from ea_common import OUT, attempts, kernel_dir, retest_path
from gen_desc import DESC, FAULT_RE, build, check, crash_site, fail_lines

ONE = {"mult": "{8, 16, 64, 1024}", "tail": "{1, 3, 6, 7, 13, 67, 1003}"}
SQUARE = {"mult": "{8, 16, 64, 256}", "tail": "{1, 3, 6, 7, 13, 67, 255}"}
TWO = {"mult": "{{8, 8}, {16, 24}, {64, 32}, {256, 128}}",
       "tail": "{{1, 1}, {3, 5}, {6, 7}, {13, 7}, {67, 35}, {255, 129}}"}
STENCIL = {"mult": "{{1, 10}, {2, 18}, {2, 34}}", "tail": "{{1, 3}, {2, 4}, {2, 5}, {1, 9}, {2, 13}, {2, 37}}"}
SWEEP = {
    **dict.fromkeys(["axpy", "dot", "scal", "nrm2", "prefix_sum", "popcount"], ONE),
    **dict.fromkeys(["gemver", "gesummv", "mvt", "trmv"], SQUARE),
    **dict.fromkeys(["atax", "bicg", "syrk", "trmm", "histogram", "lower_bound"], TWO),
    "jacobi1d": {"mult": "{{1, 10}, {2, 66}, {3, 514}}", "tail": "{{1, 3}, {2, 4}, {2, 5}, {1, 9}, {2, 67}, {3, 515}}"},
    "jacobi2d": STENCIL,
    "heat3d": STENCIL,
    "gemm": {"mult": "{{8, 8, 8}, {16, 24, 8}, {32, 40, 48}}",
             "tail": "{{1, 1, 1}, {3, 5, 7}, {7, 6, 5}, {13, 7, 9}, {33, 41, 47}}"},
    "2mm": {"mult": "{{8, 8, 8, 8}, {16, 24, 8, 32}, {32, 40, 48, 16}}",
            "tail": "{{1, 1, 1, 1}, {3, 5, 7, 6}, {7, 6, 5, 3}, {13, 7, 9, 11}, {33, 41, 47, 21}}"},
    "3mm": {"mult": "{{8, 8, 8, 8, 8}, {16, 24, 8, 32, 16}, {32, 40, 48, 16, 24}}",
            "tail": "{{1, 1, 1, 1, 1}, {3, 5, 7, 6, 4}, {7, 6, 5, 3, 9}, {13, 7, 9, 11, 5}, {33, 41, 47, 21, 35}}"},
}
SIZES_RE = re.compile(r"(static const long SIZES\[\](?:\[\d+\])? = )\{.*?\};")


def make_drivers():
    for kernel, sets in SWEEP.items():
        src = open(os.path.join(DESC, kernel, "driver.c")).read()
        for name, init in sets.items():
            d = kernel_dir(kernel, name)
            os.makedirs(d, exist_ok=True)
            text, n = SIZES_RE.subn(lambda m: m.group(1) + init + ";", src)
            assert n == 1, kernel
            open(os.path.join(d, "driver.c"), "w").write(text)
            shutil.copy(os.path.join(DESC, kernel, "ref.c"), d)


def run_tests(asm, kdir, exe):
    stage, sizes, details, info = check(asm, kdir, exe, diag=True)
    where, text = crash_site(details, asm, kdir, exe) if stage == "crash" else (None, "")
    fault = FAULT_RE.search(details)
    return {"stage": stage, "sizes": sizes, "signal": info.get("signal"), "fails": fail_lines(details, 20),
            "crash_where": where, "crash_text": text,
            "fault": dict(zip(["signal", "size", "ip", "addr"], fault.groups())) if fault else None}


def retest(a):
    dest = retest_path(a)
    if os.path.exists(dest) or not os.path.exists(a["asm"]):
        return
    res = {}
    with tempfile.TemporaryDirectory() as tmp:
        if a["stage"] in ("assemble_fail", "link_fail"):
            res["build"] = build(a["asm"], kernel_dir(a["kernel"]), os.path.join(tmp, "x"))[1][-8000:]
        elif a["stage"] in ("wrong", "crash", "timeout"):
            for name in ["orig"] + (["mult", "tail"] if a["kernel"] in SWEEP else []):
                res[name] = run_tests(a["asm"], kernel_dir(a["kernel"], None if name == "orig" else name),
                                      os.path.join(tmp, name))
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    with open(dest + ".tmp", "w") as f:
        json.dump(res, f)
    os.replace(dest + ".tmp", dest)


def check_drivers():
    with tempfile.TemporaryDirectory() as tmp:
        for kernel in SWEEP:
            for name in ("mult", "tail"):
                r = run_tests(os.path.join(DESC, kernel, "hand.S"), kernel_dir(kernel, name), os.path.join(tmp, kernel + name))
                print(kernel, name, r["stage"], " ".join(r["sizes"]))


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--jobs", type=int, default=24)
    p.add_argument("--check-drivers", action="store_true", help="only build the sweep drivers and test them on hand.S")
    args = p.parse_args()
    make_drivers()
    if args.check_drivers:
        return check_drivers()
    todo = [{k: v for k, v in a.items() if k != "rec"} for a in attempts() if not os.path.exists(retest_path(a))]
    print(f"{len(todo)} attempts to re-test", flush=True)
    with Pool(args.jobs) as pool:
        for i, _ in enumerate(pool.imap_unordered(retest, todo, chunksize=4), 1):
            if i % 500 == 0 or i == len(todo):
                print(f"{i}/{len(todo)}", flush=True)


if __name__ == "__main__":
    main()
