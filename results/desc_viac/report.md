# The "via C" baseline: same task, but the model writes C

**Question.** When a model fails to write the assembly, is the cause the algorithm (understanding the description) or the target (x86-64 syntax, the ABI, registers, vector code)? To find out, the model gets the same task in `desc_viac`, but writes C, and gcc compiles it. Everything else is identical to `desc_full`:
- the same 24 kernels, levels L0–L3, 5 repeats and 4 models;
- the same tests and up to 5 repair rounds with compiler and test feedback.

## How the numbers are produced

All tables are generated. The commands below reproduce the files in this directory (`viac.md` is checked to be byte-identical between runs):

```
python3 scripts/desc_compare.py results/desc_full results/desc_viac > results/desc_viac/compare.md
python3 scripts/desc_viac.py   results/desc_full results/desc_viac > results/desc_viac/viac.md
```

- **`compare.md`** pairs the two runs by model, kernel, level and repeat (1920 cells, all answered in both runs). It reports the pass rates with C − asm and a 95% CI from a bootstrap over kernels, the failure mix of the first answers, and cost and time per cell.
- **`viac.md`** has:
  - the failure stages that exist only for C;
  - the most common compiler errors;
  - the offline re-score of unclosed code blocks (see below);
  - per-kernel and per-level tables;
  - the cells that still fail after repair.

  The script rebuilds and tests saved answers from `results/<run>/raw`. It makes no API calls.

Setup of `desc_viac`, from its `RUN.txt`:
- `--target c`, gcc `-O2 -march=x86-64-v3`;
- prompt `prompts/desc_c.txt`: plain C11, no inline assembly or intrinsics, no `main`, only `<math.h>` from the standard library;
- Qwen3.8-27B with reasoning off (`none`), as in `desc_full`.

## Main result

| model | asm, first try | C, first try | asm, after repair | C, after repair |
|---|---|---|---|---|
| Qwen3.8-27B (reasoning off) | 9% | 98% | 34% | 99% |
| DeepSeek-V4-Flash | 20% | 95%* | 52% | 99% |
| GLM-5.3-Flash | 32% | 89% | 83% | 100% |
| Kimi-K3 | 86% | 99% | 99% | 100% |
| **all** | **37%** | **95%*** | **67%** | **99%** |

\* With unclosed code blocks re-scored (next section). As recorded, DeepSeek C first try is 76% and the overall C first try is 90%.

- **C − asm, all models** (`viac.md`, 95% CI over kernels):
  - first try: **+58 pts [+56, +62]**, or +54 [+51, +56] as recorded;
  - after repair: **+32 pts [+28, +37]**.
- **All four models write correct C for almost every task**, including the two weaker ones, whose asm mostly fails (Qwen 9% → 98%, DeepSeek 20% → 95% on first try). So the models understand the descriptions. What they lack is the step from a correct algorithm to correct x86-64 assembly.
- **Strong models close the gap themselves; weak ones don't.** Kimi already passes 86% in asm, and 99% after repair, so C adds little (+12 pts first try, +1 after repair). For Qwen, asm reaches only 34% even after 5 repairs, against 99% in C.
- **The description level hardly matters for C** (all models, re-scored): 97% first try at L0 and 94–95% at L1–L3, and 99–100% after repair at every level. That matches the null result for the level in the asm runs.
- **No kernel is hard in C** (per-kernel table in `viac.md`):
  - The lowest first-try rates are trmm 71%, gemver 82%, heat3d 88% and gemm 88%.
  - After repair, every kernel is at 95–100%.
  - In asm, every kernel stays between 25% and 55% on first try.
- **C is cheaper and faster** (`compare.md`): 1.0–1.3 calls per cell instead of 1.1–3.4, and a fraction of the cost per solved cell (e.g. Kimi $0.014 vs $0.095, DeepSeek $0.0005 vs $0.015).

## Harness issue: unclosed code blocks

DeepSeek often opens its answer with a C code block and never closes it. The extractor (`extract_code()` in `gen_desc.py`) finds no complete block, then takes the whole reply as code, including the opening fence line. gcc therefore fails at line 1 with "stray ‘`’ in program".

- **This affects 106 DeepSeek first answers in `desc_viac`** (107 records in all rounds), so 22% of DeepSeek's first tries were recorded as `compile_fail`.
- **Re-scored with the block closed** (the same answer, built and tested again), they come out as: 92 pass, 8 wrong, 6 compile errors. That gives the corrected DeepSeek C first try of 95%.
- **In the asm run** it affects only 5 Qwen first answers. Re-scoring changes none of their outcomes (3 incomplete, 2 assembler errors), so the asm numbers stand.
- **After repair, nothing changes:** the repair round showed the compiler error and DeepSeek fixed the fence.

## What still fails in C

- **Compiler errors not caused by the fence** come mostly from the header rule in the prompt ("include only `<math.h>`"). The largest group is **GLM: 36 records with `size_t` undeclared**; Kimi has 3. GLM uses `size_t` without including `<stddef.h>`, which the prompt didn't allow. The next group is `malloc` without `<stdlib.h>` (Qwen 2, DeepSeek 2). So part of GLM's 9% compiler errors on first try comes from our prompt, not from the model's C. After repair, they're all fixed.
- **After repair, 10 of 1920 C cells still fail**, all with wrong values:
  - Qwen 3 (jacobi1d 2, sort8 1), DeepSeek 5 (trmm 3, sort8 2), GLM 2 (heat3d 1, trmm 1), Kimi 0;
  - in asm, 633 cells still fail (Qwen 317, DeepSeek 231, GLM 80, Kimi 5).
- **4 cells are solved in asm but not in C:**
  - Qwen jacobi1d L3 rep 0 and rep 3;
  - DeepSeek trmm L1 rep 2;
  - GLM trmm L2 rep 0.

## implications

- **The via-C arm separates the two parts of the task.** Algorithm: 95% correct on first try, 99% after repair. Target: 37% on first try, 67% after repair. Feedback narrows but does not close the asm gap, except for the strongest model.
- the bottleneck is target-specific knowledge, not understanding the description.

## Limitations

- **The prompt allows only `<math.h>`.** Answers that rely on `size_t` or `malloc` without the matching header fail to compile. This lowers C first try for GLM in particular. It doesn't affect the after-repair results.
- **The C answers go through the unchanged extractor.** The unclosed-block issue is corrected only in the re-scored tables. The recorded records are unchanged.
- **gcc `-O2 -march=x86-64-v3` is the only compiler setting.** A failure that only shows up with other flags or another compiler would be missed.
- **One run per arm, with the gateway's default sampling and a random seed**, as in `desc_full`. 
