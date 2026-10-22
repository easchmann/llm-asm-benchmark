#!/usr/bin/env python3
# pgfplots figures for a description->asm run, written as .tex files plus the data as .csv
# usage: python3 scripts/desc_plots.py results/desc_full [--out results/desc_full/plots]
# in the paper: \input{plots/preamble.tex} in the preamble, \input{plots/<figure>.tex} inside a figure

import argparse
import collections
import csv
import os

from desc_repair import CATEGORY, trajectories
from gen_desc import load_records

LEVELS = ["L0", "L1", "L2", "L3"]
MODELS = [
    ("moonshotai/Kimi-K3", "Kimi-K3", "modelA"),
    ("zai-org/GLM-5.3-Flash", "GLM-5.3-Flash", "modelB"),
    ("deepseek-ai/DeepSeek-V4-Flash", "DeepSeek-V4-Flash", "modelC"),
    ("Qwen/Qwen3.8-27B@reasoning-none", "Qwen3.8-27B (no thinking)", "modelD"),
]
CATEGORIES = [("blas1", "BLAS-1"), ("blas2", "BLAS-2"), ("blas3", "BLAS-3"),
              ("stencil", "stencil"), ("integer", "integer"), ("irregular", "irregular")]
ROUND_BINS = [("round 0 (first try)", "rnd0"), ("round 1", "rnd1"), ("round 2", "rnd2"),
              ("round 3", "rnd3"), ("round 4--5", "rnd4"), ("not solved", "unsolved")]
C_FAILURE_LABELS = {"failA": "compiler error", "failF": "other (link, forbidden construct)"}
FAILURES = [("assembler error", "failA"), ("crash or timeout", "failB"),
            ("ABI or memory violation", "failC"), ("wrong values, larger sizes only", "failD"),
            ("wrong values, all sizes", "failE"), ("other (link, unfinished)", "failF")]

PREAMBLE = r"""\usepackage{pgfplots}
\usepgfplotslibrary{groupplots}
\usetikzlibrary{patterns, calc}
\pgfplotsset{compat=1.18}
\definecolor{modelA}{HTML}{2A78D6}
\definecolor{modelB}{HTML}{EB6834}
\definecolor{modelC}{HTML}{1BAF7A}
\definecolor{modelD}{HTML}{EDA100}
\definecolor{failA}{HTML}{2A78D6}
\definecolor{failB}{HTML}{EB6834}
\definecolor{failC}{HTML}{1BAF7A}
\definecolor{failD}{HTML}{EDA100}
\definecolor{failE}{HTML}{E87BA4}
\definecolor{failF}{HTML}{008300}
\definecolor{rnd0}{HTML}{0D366B}
\definecolor{rnd1}{HTML}{184F95}
\definecolor{rnd2}{HTML}{256ABF}
\definecolor{rnd3}{HTML}{3987E5}
\definecolor{rnd4}{HTML}{86B6EF}
\definecolor{unsolved}{HTML}{C3C2B7}
\definecolor{inksecondary}{HTML}{52514E}
\definecolor{legendgray}{HTML}{6B6A65}
\definecolor{gridline}{HTML}{E1E0D9}
\definecolor{baseline}{HTML}{C3C2B7}
\pgfplotsset{
  descaxis/.style={
    axis lines*=left, axis line style={baseline}, tick style={baseline},
    ymajorgrids, grid style={gridline, line width=0.4pt},
    tick label style={font=\scriptsize, text=inksecondary},
    label style={font=\footnotesize, text=inksecondary},
    title style={font=\footnotesize},
    legend style={draw=none, fill=none, font=\scriptsize, text=inksecondary},
    legend cell align=left, enlarge x limits=0.15,
    xtick={0,1,2,3}, xticklabels={L0,L1,L2,L3},
    ymin=0, ymax=100, ytick={0,25,50,75,100},
  },
  firsttry/.style={fill=#1, draw=white, line width=0.6pt},
  repaired/.style={fill=#1!30, draw=white, line width=0.6pt,
                   postaction={pattern=north east lines, pattern color=#1}},
}
"""


def pct(k, n):
    return 100.0 * k / n if n else 0.0


def classify(r):
    stage = r["stage"]
    if stage in ("assemble_fail", "compile_fail"):
        return "failA"
    if stage in ("crash", "timeout"):
        return "failB"
    if stage != "wrong":
        return "failF"
    text = " ".join(r.get("test_failures") or [])
    if any(s in text for s in ("callee-saved", "stack pointer", "outside the array", "input was modified")):
        return "failC"
    sizes = list((r.get("sizes") or {}).values())
    return "failD" if sizes and sizes[0] == "pass" and "fail" in sizes else "failE"


def solved_bin(rounds):
    hit = next((r["round"] for r in rounds if r["stage"] == "pass"), None)
    return "unsolved" if hit is None else f"rnd{min(hit, 4)}"


def tex_name(text):
    return text.replace("_", r"\_")


def coords(points):
    return " ".join(f"({x:g},{y:.2f})" for x, y in points)


def bar_pair(color, shift, width, first, final, label=False):
    nodes = (", nodes near coords={\\pgfmathprintnumber[fixed,precision=0]{\\pgfplotspointmeta}}, "
             "nodes near coords style={font=\\tiny, text=inksecondary}, point meta=y") if label else ""
    return (f"  \\addplot[forget plot, repaired={color}, bar shift={shift:g}, bar width={width:g}{nodes}] "
            f"coordinates {{{coords(final)}}};\n"
            f"  \\addplot[forget plot, firsttry={color}, bar shift={shift:g}, bar width={width:g}] "
            f"coordinates {{{coords(first)}}};\n")


def model_legend():
    lines = "".join(f"  \\addlegendimage{{area legend, firsttry={c}}}\\addlegendentry{{{name}}}\n"
                    for _, name, c in MODELS)
    return (lines + "  \\addlegendimage{area legend, firsttry=legendgray}\\addlegendentry{first try}\n"
                    "  \\addlegendimage{area legend, repaired=legendgray}\\addlegendentry{added by repair}\n")


def write(out, name, body):
    with open(os.path.join(out, f"{name}.tex"), "w") as f:
        f.write(body)
    print(f"wrote {os.path.join(out, name + '.tex')}")


def fig_levels(rate, out):
    width = 0.18
    body = ("\\begin{tikzpicture}\n\\begin{axis}[descaxis, ybar, width=\\linewidth, height=0.5\\linewidth,\n"
            "  ymax=112, enlarge x limits={abs=0.6}, ylabel={pass rate (\\%)}, xlabel={description level},\n"
            "  legend columns=3, legend style={at={(0.5,1.03)}, anchor=south}]\n")
    for i, (model, _, color) in enumerate(MODELS):
        shift = (i - (len(MODELS) - 1) / 2) * (width + 0.02)
        first = [(j, rate[model, l][0]) for j, l in enumerate(LEVELS)]
        final = [(j, rate[model, l][1]) for j, l in enumerate(LEVELS)]
        body += bar_pair(color, shift, width, first, final, label=True)
    body += model_legend() + "\\end{axis}\n\\end{tikzpicture}\n"
    write(out, "pass_by_level", body)


def fig_kernels(rate, kernels, out):
    width = 0.18
    body = ("\\begin{tikzpicture}\n\\begin{groupplot}[descaxis, ybar, group style={group size=4 by 6,\n"
            "  horizontal sep=0.6cm, vertical sep=1.1cm, y descriptions at=edge left},\n"
            "  width=0.31\\linewidth, height=0.24\\linewidth, ytick={0,50,100},\n"
            "  enlarge x limits={abs=0.6}, title style={yshift=-1.5ex}, ylabel={pass rate (\\%)}]\n")
    for k, (kernel, category) in enumerate(kernels):
        legend = ", legend to name=kernellegend, legend columns=3" if k == 0 else ""
        body += f"\\nextgroupplot[title={{{tex_name(kernel)} \\textcolor{{inksecondary}}{{({category})}}}}{legend}]\n"
        for i, (model, _, color) in enumerate(MODELS):
            shift = (i - (len(MODELS) - 1) / 2) * (width + 0.02)
            first = [(j, rate[model, kernel, l][0]) for j, l in enumerate(LEVELS)]
            final = [(j, rate[model, kernel, l][1]) for j, l in enumerate(LEVELS)]
            body += bar_pair(color, shift, width, first, final)
        if k == 0:
            body += model_legend()
    body += ("\\end{groupplot}\n\\node[anchor=north] at ($(group c2r6.south)!0.5!(group c3r6.south) + (0,-0.9cm)$)"
             " {\\pgfplotslegendfromname{kernellegend}};\n\\end{tikzpicture}\n")
    write(out, "pass_by_kernel", body)


def stacked_panels(out, name, shares, bins, ylabel, legend_columns, rows=1, row_titles=None):
    body = ("\\begin{tikzpicture}\n\\begin{groupplot}[descaxis, ybar stacked, /pgf/bar width=0.55,\n"
            f"  group style={{group size=4 by {rows}, horizontal sep=0.5cm, vertical sep=2.2cm, y descriptions at=edge left}},\n"
            f"  width=0.3\\linewidth, height=0.32\\linewidth, ylabel={{{ylabel}}}]\n")
    for row in range(rows):
        for m, (model, label, _) in enumerate(MODELS):
            title = label + (f"\\\\\\textcolor{{inksecondary}}{{{row_titles[row]}}}" if row_titles else "")
            legend = f", legend to name={name}legend, legend columns={legend_columns}" if row == 0 and m == 0 else ""
            body += f"\\nextgroupplot[title={{{title}}}, title style={{align=center}}{legend}]\n"
            for b_label, color in bins:
                points = [(j, shares[row][model, l][color]) for j, l in enumerate(LEVELS)]
                body += f"  \\addplot[fill={color}, draw=white, line width=0.6pt] coordinates {{{coords(points)}}};\n"
                if row == 0 and m == 0:
                    body += f"  \\addlegendentry{{{b_label}}}\n"
    body += (f"\\end{{groupplot}}\n\\node[anchor=north] at ($(group c2r{rows}.south)!0.5!(group c3r{rows}.south) + (0,-0.9cm)$)"
             f" {{\\pgfplotslegendfromname{{{name}legend}}}};\n\\end{{tikzpicture}}\n")
    write(out, name, body)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("run_dir")
    p.add_argument("--out")
    args = p.parse_args()
    out_dir = args.out or os.path.join(args.run_dir, "plots")
    os.makedirs(out_dir, exist_ok=True)

    cells = [c for c in trajectories(load_records(args.run_dir)).values() if c[0]["stage"] != "no_answer"]
    groups = collections.defaultdict(list)
    for c in cells:
        r = c[0]
        groups[r["model"], r["level"]].append(c)
        groups[r["model"], r["kernel"], r["level"]].append(c)

    rate = {key: (pct(sum(c[0]["stage"] == "pass" for c in g), len(g)), pct(sum(c[-1]["stage"] == "pass" for c in g), len(g)))
            for key, g in groups.items()}
    rounds = {key: {color: pct(sum(solved_bin(c) == color for c in g), len(g)) for _, color in ROUND_BINS}
              for key, g in groups.items() if len(key) == 2}
    failures = [{key: {color: pct(sum(c[i]["stage"] != "pass" and classify(c[i]) == color for c in g), len(g))
                       for _, color in FAILURES}
                 for key, g in groups.items() if len(key) == 2} for i in (0, -1)]

    with open(os.path.join(out_dir, "preamble.tex"), "w") as f:
        f.write(PREAMBLE)
    with open(os.path.join(out_dir, "data.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["model", "kernel", "level", "cells", "pass_first_try_pct", "pass_after_repair_pct"])
        for key, g in sorted(groups.items()):
            model, kernel, level = key if len(key) == 3 else (key[0], "all", key[1])
            w.writerow([model, kernel, level, len(g), f"{rate[key][0]:.1f}", f"{rate[key][1]:.1f}"])

    kernels = sorted({(c[0]["kernel"], CATEGORY[c[0]["kernel"]]) for c in cells},
                     key=lambda kc: ([k for k, _ in CATEGORIES].index(kc[1]), kc[0]))
    labels = dict(CATEGORIES)
    fig_levels(rate, out_dir)
    fig_kernels(rate, [(k, labels[c]) for k, c in kernels], out_dir)
    stacked_panels(out_dir, "repair_rounds", [rounds], ROUND_BINS, "attempts (\\%)", 6)
    bins = FAILURES
    if any(c[0].get("mode") == "c" for c in cells):
        bins = [(C_FAILURE_LABELS.get(code, label), code) for label, code in FAILURES]
    stacked_panels(out_dir, "failures_by_level", failures, bins, "attempts failing (\\%)", 3, rows=2,
                   row_titles=["first try", "after repair"])


if __name__ == "__main__":
    main()
