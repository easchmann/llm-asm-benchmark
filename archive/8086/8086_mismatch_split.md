# 8086 "mismatch" results (r1–r4)

Raw data: `results/analysis/8086_mismatch_split.csv` (every `compiled=yes` 8086 cell, 14 total across repeats 1–4).

## Observations

- **10 of 14 shipped outputs ARE real numeric outputs and all of them are wrong.** Nearly every dumped value is `0.00` (79–80 of 80 values) or a huge garbage number (a ~300-digit value in r1 atax/Qwen3.8, uninitialized memory). The generated asm runs but never writes the result array.
- **4 of 14 are not model outputs at all** — they're QEMU/FreeDOS boot artifacts: 3 empty files (0 B) and 1 binary 25 B blob. They got graded `mismatch` only because the current pipeline doesn't check whether the boot produced a real `DUMP_ARRAYS` dump.
- **The only 8086 pass in the whole run is genuine**: r3 atax/Kimi-K3 — its dumped values match native exactly.
- **Who fails which way** (observed split):
  - Qwen3.8-27B: 3 real outputs, all wrong (zeros/garbage).
  - GLM-5.3-Flash: 3 real outputs (zeros), plus 3 empty/binary artifacts.
  - Kimi-K3: 1 pass (atax r3), 3 real outputs wrong (zeros), 1 empty artifact.
- **Correctness feedback has nothing to grab here**: these are "no output written," not "one wrong value". This might also be why the second repair loop fails.

## Bottom line

On 8086, "mismatch" is really two things: **unwritten-buffer dumps** (model failure: compiles, runs, stores nothing) and **boot-harness junk** (measurement failure). A `DUMP_ARRAYS`-present guard in the 8086 runner would reclassify the 4 artifacts as `no_output`.