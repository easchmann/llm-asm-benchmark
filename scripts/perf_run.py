#!/usr/bin/env python3
# time passing answers of desc runs and the baselines with kernels/desc/*/bench.c, one pinned core, resumable
# programs are deduplicated by machine code (stripped object); results: OUT/programs.jsonl, OUT/timings.jsonl, OUT/run.log
# usage: python3 scripts/perf_run.py RUN_DIR... [--core 20] [--out results/perf]
#        python3 scripts/perf_run.py --estimate RUN_DIR...   count programs and estimate the time
#        python3 scripts/perf_run.py --selftest              baselines and hand.S pass both sizes, mutants are caught
#        python3 scripts/perf_run.py --stability             same binary measured repeatedly, also next to a busy neighbour

import argparse
import collections
import glob
import hashlib
import json
import os
import platform
import random
import resource
import shutil
import signal
import statistics
import subprocess
import sys
import tempfile
import time

from desc_repair import trajectories
from gen_desc import C_FLAGS, DESC, ROOT, load_records

SIZES = ["small", "large"]
BASELINES = {
    "gcc-O2-v3": ["gcc", "-O2", "-march=x86-64-v3"],
    "gcc-O2-scalar": ["gcc", "-O2", "-march=x86-64"],
    "gcc-O3-native": ["gcc", "-O3", "-march=native"],
    "gcc-O3-native-fastmath": ["gcc", "-O3", "-march=native", "-ffast-math"],
    "clang-O3-native": ["clang", "-O3", "-march=native"],
}
REFERENCE = "gcc-O2-v3"
TIMEOUT, CPU_SECONDS, MEM_MB = 120, 300, 4000
MAX_TRIES, CANARY_EVERY, DRIFT, RECHECKS = 5, 50, 0.02, 50
WARMUP_SECONDS = 60
SIBLING_BUSY, NEIGHBOUR_BUSY, NODE_BUSY = 0.05, 0.10, 0.25
STRIP = ["--strip-all", "-R", ".comment", "-R", ".note.GNU-stack", "-R", ".note.gnu.property"]


def kernels():
    return sorted(os.path.basename(os.path.dirname(p)) for p in glob.glob(os.path.join(DESC, "*", "bench.c")))


def harness_id(kernel):
    h = hashlib.sha256()
    for path in (os.path.join(DESC, kernel, "bench.c"), os.path.join(DESC, kernel, "ref.c"),
                 os.path.join(DESC, "common", "bench.h"), os.path.join(DESC, "common", "check.h")):
        h.update(open(path, "rb").read())
    return h.hexdigest()[:12]


def function_name(kernel):
    return json.load(open(os.path.join(DESC, kernel, "spec.json")))["name"]


def cpu_list(path):
    cpus = []
    for part in open(path).read().strip().split(","):
        lo, _, hi = part.partition("-")
        cpus += range(int(lo), int(hi or lo) + 1)
    return cpus


def topology(core):
    base = f"/sys/devices/system/cpu/cpu{core}"
    node = int(next(d for d in os.listdir(base) if d.startswith("node") and d[4:].isdigit())[4:])
    siblings = [c for c in cpu_list(f"{base}/topology/thread_siblings_list") if c != core]
    l3 = [c for c in cpu_list(f"{base}/cache/index3/shared_cpu_list") if c != core and c not in siblings]
    node_cpus = cpu_list(f"/sys/devices/system/node/node{node}/cpulist")
    others = [c for c in node_cpus if c != core and c not in siblings and c not in l3]
    return node, siblings, l3, others


def cpu_times():
    times = {}
    for line in open("/proc/stat"):
        f = line.split()
        if f[0].startswith("cpu") and f[0][3:].isdigit():
            v = list(map(int, f[1:]))
            times[int(f[0][3:])] = (sum(v), v[3] + v[4])
    return times


def busy(before, after, cpus):
    out = []
    for c in cpus:
        total = after[c][0] - before[c][0]
        out.append(0.0 if total <= 0 else 1 - (after[c][1] - before[c][1]) / total)
    return out


def tool(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True).stdout


def environment(core, node):
    cpuinfo = open("/proc/cpuinfo").read().splitlines()
    field = lambda key: next((l.split(":", 1)[1].strip() for l in cpuinfo if l.startswith(key)), None)
    read = lambda p: open(p).read().strip() if os.path.exists(p) else None
    return {"host": platform.node(), "core": core, "node": node, "cpu": field("model name"),
            "microcode": field("microcode"), "kernel": platform.release(),
            "gcc": tool("gcc", "--version").splitlines()[0], "as": tool("as", "--version").splitlines()[0],
            "clang": (tool("clang", "--version").splitlines() or [None])[0],
            "thp": read("/sys/kernel/mm/transparent_hugepage/enabled"),
            "boost": read("/sys/devices/system/cpu/cpufreq/boost"),
            "governor": read(f"/sys/devices/system/cpu/cpu{core}/cpufreq/scaling_governor")}


class Builder:
    def __init__(self, out):
        self.obj_dir = os.path.join(out, "code", "obj")
        self.bin_dir = os.path.join(out, "code", "bin")
        self.harness_dir = os.path.join(out, "code", "harness")
        for d in (self.obj_dir, self.bin_dir, self.harness_dir):
            os.makedirs(d, exist_ok=True)
        self.tmp = tempfile.mkdtemp(prefix="perf_build_")
        self.harness = {}

    def compile(self, cmd, src):
        tmp = os.path.join(self.tmp, "x.o")
        r = subprocess.run([*cmd, "-c", src, "-o", tmp], capture_output=True, text=True)
        if r.returncode:
            return None, r.stderr[-2000:]
        stripped = os.path.join(self.tmp, "s.o")
        subprocess.run(["objcopy", *STRIP, tmp, stripped], check=True)
        h = hashlib.sha256(open(stripped, "rb").read()).hexdigest()[:16]
        obj = os.path.join(self.obj_dir, h + ".o")
        if not os.path.exists(obj):
            shutil.move(tmp, obj)
        return h, ""

    def answer(self, path):
        return self.compile(["gcc", *C_FLAGS] if path.endswith(".c") else ["gcc"], path)

    def baseline(self, kernel, variant):
        name = function_name(kernel)
        return self.compile([*BASELINES[variant], f"-Dref_{kernel}={name}"], os.path.join(DESC, kernel, "ref.c"))

    def exe(self, kernel, h):
        path = os.path.join(self.bin_dir, kernel, harness_id(kernel), h)
        if os.path.exists(path):
            return path, ""
        if kernel not in self.harness:
            d = os.path.join(self.harness_dir, kernel)
            os.makedirs(d, exist_ok=True)
            objs = []
            for src in ("bench.c", "ref.c"):
                obj = os.path.join(d, src[:-2] + ".o")
                subprocess.run(["gcc", "-O2", "-I", os.path.join(DESC, "common"), "-c",
                                os.path.join(DESC, kernel, src), "-o", obj], check=True)
                objs.append(obj)
            self.harness[kernel] = objs
        os.makedirs(os.path.dirname(path), exist_ok=True)
        r = subprocess.run(["gcc", *self.harness[kernel], os.path.join(self.obj_dir, h + ".o"), "-o", path, "-lm"],
                           capture_output=True, text=True)
        return (path, "") if r.returncode == 0 else (None, r.stderr[-2000:])


def limits():
    resource.setrlimit(resource.RLIMIT_CPU, (CPU_SECONDS, CPU_SECONDS))
    resource.setrlimit(resource.RLIMIT_AS, (MEM_MB << 20, MEM_MB << 20))
    resource.setrlimit(resource.RLIMIT_FSIZE, (1 << 20, 1 << 20))


class Machine:
    def __init__(self, core):
        self.core = core
        self.node, self.siblings, self.l3, self.others = topology(core)

    def run(self, exe, size, check=False):
        cmd = ["taskset", "-c", str(self.core), "numactl", "--membind", str(self.node), exe, size]
        before, start = cpu_times(), time.time()
        try:
            p = subprocess.run(cmd + (["check"] if check else []), capture_output=True, text=True,
                               timeout=TIMEOUT, preexec_fn=limits)
            rc, out = p.returncode, p.stdout
        except subprocess.TimeoutExpired as e:
            rc, out = None, (e.stdout or b"").decode(errors="replace") if isinstance(e.stdout, bytes) else (e.stdout or "")
        after = cpu_times()
        lines = out.strip().splitlines()
        row = {}
        if rc is None:
            row["status"] = "timeout"
        elif rc < 0:
            row["status"] = "crash"
            row["signal"] = signal.Signals(-rc).name
        elif lines and lines[-1].startswith("{"):
            row = json.loads(lines[-1])
        else:
            row["status"] = "error"
        if rc == -signal.SIGXCPU:
            row["status"] = "timeout"
        fails = [l for l in lines if ": fail " in l]
        if fails:
            row["detail"] = fails[:5]
        sib = busy(before, after, self.siblings)
        l3 = busy(before, after, self.l3)
        others = busy(before, after, self.others)
        row["busy_sibling"] = round(max(sib, default=0), 3)
        row["busy_l3"] = round(max(l3, default=0), 3)
        row["busy_node"] = round(max(others, default=0), 3)
        row["contended"] = (row["busy_sibling"] > SIBLING_BUSY or row["busy_l3"] > NEIGHBOUR_BUSY
                            or row["busy_node"] > NODE_BUSY)
        row["wall_s"] = round(time.time() - start, 2)
        return row


def warm_up(builder, machine, log, seconds=WARMUP_SECONDS):
    exes = [builder.exe(k, builder.baseline(k, REFERENCE)[0])[0] for k in kernels()]
    end, n = time.time() + seconds, 0
    while time.time() < end:
        machine.run(exes[n % len(exes)], SIZES[n // len(exes) % 2])
        n += 1
    log(f"warm-up: {n} discarded runs in {seconds} s")


def answer_file(run_dir, r):
    base = os.path.join(run_dir, "code", f"{r['cell']}.r{r['round']}")
    return next((base + e for e in (".S", ".c") if os.path.exists(base + e)), None)


def collect(run_dirs, builder, log):
    rows = []
    for kernel in kernels():
        for variant in BASELINES:
            h, err = builder.baseline(kernel, variant)
            rows.append({"run": "baseline", "cell": f"{kernel}__{variant}", "kernel": kernel, "variant": variant,
                         "lang": "c", "hash": h, "build_error": err or None})
        h, err = builder.answer(os.path.join(DESC, kernel, "hand.S"))
        rows.append({"run": "baseline", "cell": f"{kernel}__hand", "kernel": kernel, "variant": "hand",
                     "lang": "asm", "hash": h, "build_error": err or None})
    for run_dir in run_dirs:
        run = os.path.basename(os.path.normpath(run_dir))
        cells = trajectories(load_records(run_dir))
        passing = [rounds[-1] for rounds in cells.values() if rounds[-1]["stage"] == "pass"]
        log(f"{run}: {len(cells)} cells, {len(passing)} passing")
        for r in passing:
            path = answer_file(run_dir, r)
            h, err = builder.answer(path) if path else (None, "answer file missing")
            rows.append({"run": run, "run_dir": os.path.abspath(run_dir), "cell": r["cell"], "kernel": r["kernel"],
                         "level": r["level"], "model": r["model"], "repeat": r["repeat"], "round": r["round"],
                         "lang": "c" if path and path.endswith(".c") else "asm", "file": path, "hash": h,
                         "build_error": err or None})
    return rows


def save_programs(out, rows):
    path = os.path.join(out, "programs.jsonl")
    runs = {r["run"] for r in rows}
    old = [json.loads(l) for l in open(path)] if os.path.exists(path) else []
    keep = [r for r in old if r["run"] not in runs]
    with open(path + ".tmp", "w") as f:
        for r in keep + rows:
            f.write(json.dumps(r) + "\n")
    os.replace(path + ".tmp", path)


def load_timings(out):
    path = os.path.join(out, "timings.jsonl")
    return [json.loads(l) for l in open(path) if l.strip()] if os.path.exists(path) else []


def valid_rows(rows):
    dropped = {(r["session"], r["block"]) for r in rows if r.get("type") == "invalidate"}
    current = {k: harness_id(k) for k in kernels()}
    return [r for r in rows if r.get("type") == "timing" and (r["session"], r["block"]) not in dropped
            and r.get("harness") == current.get(r["kernel"])]


def final(r):
    return not r["contended"] or r["try"] >= MAX_TRIES


def estimate(jobs):
    per = collections.Counter()
    for kernel, size in ((k, s) for _, k, s in jobs):
        blas3 = kernel in ("gemm", "2mm", "3mm", "syrk", "trmm")
        per[kernel] += (15 if size == "large" else 2) if blas3 else (1.5 if size == "large" else 1)
    return sum(per.values())


def selftest(builder, machine):
    ok = True
    print("| kernel | program | small | large |\n|---|---|---|---|")
    for kernel in kernels():
        programs = [(v, builder.baseline(kernel, v)[0]) for v in BASELINES]
        programs.append(("hand", builder.answer(os.path.join(DESC, kernel, "hand.S"))[0]))
        for m in sorted(glob.glob(os.path.join(DESC, kernel, "mutants", "*.S"))):
            programs.append(("mutant " + os.path.basename(m)[:-2], builder.answer(m)[0]))
        for label, h in programs:
            exe, err = builder.exe(kernel, h) if h else (None, "build failed")
            status = [machine.run(exe, s, check=True)["status"] if exe else "build_fail" for s in SIZES]
            good = status[1] != "ok" if label.startswith("mutant") else status == ["ok", "ok"]
            ok &= good
            print(f"| {kernel} | {label} | {status[0]} | {status[1]} |" + ("" if good else " **BAD**"))
    print("\nall good" if ok else "\nFAILED")
    return ok


def stability(builder, machine, out, repeats=20):
    cases = [("axpy", "small"), ("axpy", "large"), ("gemm", "small"), ("dot", "large"), ("spmv", "large"),
             ("jacobi2d", "small")]
    exes = {k: builder.exe(k, builder.baseline(k, REFERENCE)[0])[0] for k, _ in cases}
    path = os.path.join(out, "stability.jsonl")

    def series(label, n):
        res = collections.defaultdict(list)
        for _ in range(n):
            for k, s in cases:
                r = machine.run(exes[k], s)
                res[k, s].append(r)
                with open(path, "a") as f:
                    f.write(json.dumps({"label": label, "kernel": k, "size": s, **r}) + "\n")
        return res

    def cv(x):
        return 100 * statistics.stdev(x) / statistics.mean(x)

    cold = series("cold", 1)
    warm_up(builder, machine, print)
    quiet = series("quiet", repeats)
    print(f"## Stability: {REFERENCE} build, {repeats} separate processes per case, core {machine.core}\n")
    print("| kernel | size | median ms | CV of median | CV of min | range of medians | contended runs |")
    print("|---|---|---|---|---|---|---|")
    for (k, s), rs in quiet.items():
        med, mn = [r["med_ns"] for r in rs], [r["min_ns"] for r in rs]
        print(f"| {k} | {s} | {statistics.mean(med) / 1e6:.4g} | {cv(med):.2f}% | {cv(mn):.2f}% | "
              f"{100 * (max(med) - min(med)) / min(med):.1f}% | {sum(r['contended'] for r in rs)} |")
    print("\n| kernel | size | first run before warm-up vs warm median |\n|---|---|---|")
    for (k, s), rs in cold.items():
        q = statistics.median(r["med_ns"] for r in quiet[k, s])
        print(f"| {k} | {s} | {100 * (rs[0]['med_ns'] / q - 1):+.1f}% |")
    hog = builder.exe("axpy", builder.baseline("axpy", REFERENCE)[0])[0]
    print("\n| neighbour load | kernel | size | median vs quiet | min vs quiet | flagged contended |")
    print("|---|---|---|---|---|---|")
    for label, cpu, size in [("SMT sibling, compute", machine.siblings[0], "small"),
                             ("same L3, memory stream", machine.l3[0], "large"),
                             ("same NUMA node, memory stream", machine.others[0], "large")]:
        load = subprocess.Popen(["bash", "-c", f"while :; do taskset -c {cpu} numactl --membind {machine.node} "
                                 f"{hog} {size} >/dev/null; done"], start_new_session=True)
        try:
            time.sleep(1)
            res = series(label, 3)
        finally:
            os.killpg(load.pid, signal.SIGTERM)
            load.wait()
        for (k, s), rs in res.items():
            q = quiet[k, s]
            med = statistics.median(r["med_ns"] for r in rs) / statistics.median(r["med_ns"] for r in q)
            mn = statistics.median(r["min_ns"] for r in rs) / statistics.median(r["min_ns"] for r in q)
            print(f"| {label} | {k} | {s} | {100 * (med - 1):+.1f}% | {100 * (mn - 1):+.1f}% | "
                  f"{sum(r['contended'] for r in rs)}/{len(rs)} |")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("runs", nargs="*")
    p.add_argument("--core", type=int, default=20)
    p.add_argument("--out", default=os.path.join(ROOT, "results", "perf"))
    p.add_argument("--seed", type=int, default=1)
    p.add_argument("--estimate", action="store_true")
    p.add_argument("--selftest", action="store_true")
    p.add_argument("--stability", action="store_true")
    args = p.parse_args()
    os.makedirs(args.out, exist_ok=True)
    machine = Machine(args.core)
    os.sched_setaffinity(0, [c for c in os.sched_getaffinity(0)
                             if c not in cpu_list(f"/sys/devices/system/node/node{machine.node}/cpulist")]
                         or os.sched_getaffinity(0))
    builder = Builder(args.out)
    if args.selftest:
        sys.exit(0 if selftest(builder, machine) else 1)
    if args.stability:
        return stability(builder, machine, args.out)

    log_path = os.path.join(args.out, "run.log")

    def log(msg):
        line = time.strftime("%Y-%m-%d %H:%M:%S ") + msg
        print(line, flush=True)
        with open(log_path, "a") as f:
            f.write(line + "\n")

    programs = collect(args.runs, builder, log)
    save_programs(args.out, programs)
    env = environment(args.core, machine.node)
    where = (env["host"], args.core)
    rows = valid_rows(load_timings(args.out))
    here = [r for r in rows if (r["host"], r["core"]) == where]
    tries = collections.Counter((r["hash"], r["kernel"], r["size"]) for r in here if r["role"] == "job")
    done = {(r["hash"], r["kernel"], r["size"]) for r in here if r["role"] == "job" and final(r)}
    answers = {(r["hash"], r["kernel"]) for r in programs if r["run"] != "baseline" and r["hash"]}
    jobs = [(h, k, s) for h, k in sorted(answers) for s in SIZES if (h, k, s) not in done]
    log(f"{len(answers)} distinct answer programs, {len(jobs)} of {2 * len(answers)} measurements to do, "
        f"estimate {estimate(jobs) / 3600:.1f} h + baselines")
    if args.estimate:
        return

    session = time.strftime("%Y%m%dT%H%M%S")
    with open(os.path.join(args.out, "timings.jsonl"), "a") as f:
        f.write(json.dumps({"type": "env", "session": session, **env}) + "\n")
    state = {"block": 0, "n": 0}
    block_jobs = []

    def measure(h, kernel, size, role, attempt=1):
        exe, err = builder.exe(kernel, h)
        row = machine.run(exe, size) if exe else {"status": "link_fail", "detail": [err], "contended": False}
        row = {"type": "timing", "session": session, "block": state["block"], "time": time.time(), "host": env["host"],
               "core": args.core, "harness": harness_id(kernel), "hash": h, "kernel": kernel, "size": size, "role": role,
               "try": attempt, **row}
        with open(os.path.join(args.out, "timings.jsonl"), "a") as f:
            f.write(json.dumps(row) + "\n")
        return row

    def measure_final(h, kernel, size, role, attempt=1):
        while True:
            row = measure(h, kernel, size, role, attempt)
            if final(row):
                return row
            attempt += 1
            time.sleep(10)

    baselines = [r for r in programs if r["run"] == "baseline" and r["hash"]]
    reference = {}
    warm_up(builder, machine, log)
    log(f"session {session}: {len(baselines) * 2} baseline measurements")
    for r in baselines:
        for s in SIZES:
            row = measure_final(r["hash"], r["kernel"], s, "baseline")
            if r["variant"] == REFERENCE:
                reference[r["kernel"], s] = row
            if row["status"] != "ok":
                log(f"baseline {r['cell']} {s}: {row['status']}")

    earlier = sorted({(r["hash"], r["kernel"], r["size"]) for r in here
                      if r["role"] == "job" and r["status"] == "ok" and final(r)})
    rng = random.Random(args.seed)
    if earlier:
        old = {(r["hash"], r["kernel"], r["size"]): r for r in here if r["role"] == "job" and r["status"] == "ok"}
        devs = []
        for key in rng.sample(earlier, min(RECHECKS, len(earlier))):
            row = measure_final(*key, "recheck")
            if row["status"] == "ok":
                devs.append(row["med_ns"] / old[key]["med_ns"] - 1)
        worst = max(abs(d) for d in devs) if devs else 0
        log(f"recheck of {len(devs)} earlier measurements: median |dev| {100 * statistics.median(map(abs, devs)):.2f}%, "
            f"max {100 * worst:.2f}%" + ("  WARNING: above 2%, earlier timings may not be comparable" if
                                         statistics.median(map(abs, devs)) > DRIFT else ""))

    rng.shuffle(jobs)
    queue = collections.deque((j, tries[j] + 1) for j in jobs)
    canaries = [(k, s) for k in kernels() for s in SIZES if (k, s) in reference and reference[k, s]["status"] == "ok"]
    rng.shuffle(canaries)
    start, total = time.time(), len(queue)
    finished = 0
    while queue:
        (h, kernel, size), attempt = queue.popleft()
        row = measure(h, kernel, size, "job", attempt)
        if final(row):
            finished += 1
            block_jobs.append((h, kernel, size))
        else:
            queue.append(((h, kernel, size), attempt + 1))
        state["n"] += 1
        eta = (time.time() - start) / max(finished, 1) * (total - finished)
        log(f"[{finished}/{total}] {kernel} {size} {h[:8]} {row['status']}"
            + (f" {row['med_ns'] / 1e6:.3f} ms" if row.get("med_ns") else "")
            + (" contended" if row["contended"] else "") + f"  eta {eta / 3600:.1f} h")
        if state["n"] % CANARY_EVERY == 0 and canaries:
            k, s = canaries[(state["n"] // CANARY_EVERY) % len(canaries)]
            ref = reference[k, s]["med_ns"]
            dev = [measure_final(reference[k, s]["hash"], k, s, "canary")["med_ns"] / ref - 1]
            if abs(dev[0]) > DRIFT:
                dev.append(measure_final(reference[k, s]["hash"], k, s, "canary")["med_ns"] / ref - 1)
            if all(abs(d) > DRIFT for d in dev):
                log(f"canary {k} {s} drifted {100 * dev[-1]:+.1f}%: re-measuring {len(block_jobs)} jobs")
                with open(os.path.join(args.out, "timings.jsonl"), "a") as f:
                    f.write(json.dumps({"type": "invalidate", "session": session, "block": state["block"]}) + "\n")
                queue.extend((j, 1) for j in block_jobs)
                finished -= len(block_jobs)
            state["block"] += 1
            block_jobs = []
    for k, s in sorted(reference):
        measure_final(reference[k, s]["hash"], k, s, "baseline_end")
    log("done")


if __name__ == "__main__":
    main()
