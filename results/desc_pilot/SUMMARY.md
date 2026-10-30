# desc_pilot: result summary

Pilot of the description-to-asm harness. Run 15–16 Oct 2026, missing cells re-run on 19 Oct 2026.
Data: `results/desc_pilot/calls.jsonl`, using the last record of each cell.

## Setup
- 4 kernels (axpy, atax, bicg, mvt) × 4 levels (L0 C, L1 pseudocode, L2 maths, L3 prose) × 2 repeats × 4 models = 128 cells.
- Models: Qwen3.8-27B, GLM-5.3-Flash, DeepSeek-V4-Flash, Kimi-K3.
- Single attempt per cell (no repair loop). x86-64 `.S` output, graded at 4 sizes per kernel.

## Data handling
- Cells that ended without an answer were re-run on 19 Oct.
- GLM-5.3-Flash, DeepSeek-V4-Flash and Kimi-K3 are complete: all 96 cells have a final record.
- Qwen3.8-27B is incomplete: 26 of 32 cells have a record, and its re-run is still going.

## Results

Pass rate over cells that got an answer:

| Model | Cells | No answer | Answered | Pass | Pass / answered |
|---|---|---|---|---|---|
| Kimi-K3 | 32 | 0 | 32 | 26 | 81% |
| GLM-5.3-Flash | 32 | 2 | 30 | 11 | 37% |
| DeepSeek-V4-Flash | 32 | 0 | 32 | 9 | 28% |
| Qwen3.8-27B (incomplete) | 26 | 18 | 8 | 2 | – |

By level (without Qwen3.8-27B):

| Level | Pass / answered |
|---|---|
| L0 (C) | 11/22 |
| L1 (pseudocode) | 12/24 |
| L2 (maths) | 10/24 |
| L3 (prose) | 13/24 |

By kernel (without Qwen3.8-27B): axpy 16/24, atax 11/24, mvt 10/22, bicg 9/24.

Pass / answered for each model and level:

| Model | L0 | L1 | L2 | L3 |
|---|---|---|---|---|
| Kimi-K3 | 6/8 | 7/8 | 6/8 | 7/8 |
| GLM-5.3-Flash | 3/6 | 2/8 | 2/8 | 4/8 |
| DeepSeek-V4-Flash | 2/8 | 3/8 | 2/8 | 2/8 |

## Observations
1. **No visible difference between levels.** Pass rates are similar from L0 to L3, for every model. With 8 cells per model and level, a moderate effect would not show up.
2. **Clear spread between models.** Kimi-K3 passes about four in five; GLM-5.3-Flash and DeepSeek-V4-Flash about one in three.
3. **Missing answers are almost only a Qwen3.8-27B problem.** For the other three models, only 2 of 96 cells ended without an answer, both GLM-5.3-Flash with HTTP 504.
   - Qwen3.8-27B produces very long reasoning (median around 42,000 tokens in earlier runs) and often hits the gateway's 600 s limit.
   - Of its 10 cells re-run so far: 4 answered (two in 350–410 s on the first attempt, two only on the third attempt), and 6 ended with a 504 on all three attempts.
4. **Failure types** (answered cells, without Qwen3.8-27B): wrong output 21, crash 13, assembler error 13, link error 1, extraction failure 0.
   - DeepSeek-V4-Flash accounts for 10 of the 13 crashes and 8 of the 13 assembler errors.
   - GLM-5.3-Flash's failures are mostly wrong output (12).
5. **Wrong results often depend on the size.** Of 25 wrong cells:
   - 9 pass only at the smallest size.
   - 5 pass at the largest size but fail a smaller one.
   - 2 have other partial patterns.
   - 9 fail at every size.
6. **Extraction works.** Every answer was extracted; 2 replies had two code blocks, and the last one was used.

## Limits
Two repeats, four kernels from one family (dense linear algebra). Harness works end to end

