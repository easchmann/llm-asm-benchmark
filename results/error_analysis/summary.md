# Failure classification

One main category per failed attempt; see prompts/error_rubric.md. A, B, X and the rule-based C/D/E are exact counts. Test failures the rules could not classify were judged for a random sample of whole cells (judge_sample.tsv) and weighted up to their run, so the remaining C-G counts are estimates. Cells: count (share of the row's failed attempts).

## Main category by run

| run | n | A | B | C | D | E | X |
|---|---|---|---|---|---|---|---|
| desc_full | 1892 | 1183 (63%) | 253 (13%) | 253 (13%) | 22 (1%) | 169 (9%) | 12 (1%) |
| desc_plan | 2324 | 1822 (78%) | 192 (8%) | 94 (4%) | 28 (1%) | 167 (7%) | 21 (1%) |
| desc_refplan | 2223 | 1717 (77%) | 149 (7%) | 205 (9%) | 9 (0%) | 103 (5%) | 40 (2%) |

## Judged categories within the sample of pending attempts (share, 95% interval)

Pending attempts are the test failures the rules could not classify. Intervals treat attempts as independent; attempts of one cell are correlated, so they are somewhat too narrow.

| run | judged | C | D | E | F | G |
|---|---|---|---|---|---|---|
| desc_full | 0 | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) |
| desc_plan | 0 | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) |
| desc_refplan | 0 | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) | 0% (0-0) |

## Main category by run and model

| run and model | n | A | B | C | D | E | X |
|---|---|---|---|---|---|---|---|
| ('desc_full', 'Qwen/Qwen3.8-27B@reasoning-none') | 987 | 791 (80%) | 61 (6%) | 98 (10%) | 16 (2%) | 12 (1%) | 9 (1%) |
| ('desc_full', 'deepseek-ai/DeepSeek-V4-Flash') | 614 | 254 (41%) | 178 (29%) | 126 (21%) | 4 (1%) | 51 (8%) | 1 (0%) |
| ('desc_full', 'moonshotai/Kimi-K3') | 50 | 20 (40%) | 1 (2%) | 1 (2%) | 0 (0%) | 28 (56%) | 0 (0%) |
| ('desc_full', 'zai-org/GLM-5.3-Flash') | 241 | 118 (49%) | 13 (5%) | 28 (12%) | 2 (1%) | 78 (32%) | 2 (1%) |
| ('desc_plan', 'Qwen/Qwen3.8-27B@reasoning-none') | 1570 | 1381 (88%) | 99 (6%) | 36 (2%) | 22 (1%) | 14 (1%) | 18 (1%) |
| ('desc_plan', 'deepseek-ai/DeepSeek-V4-Flash') | 470 | 286 (61%) | 74 (16%) | 38 (8%) | 6 (1%) | 64 (14%) | 2 (0%) |
| ('desc_plan', 'moonshotai/Kimi-K3') | 23 | 16 (70%) | 0 (0%) | 1 (4%) | 0 (0%) | 5 (22%) | 1 (4%) |
| ('desc_plan', 'zai-org/GLM-5.3-Flash') | 261 | 139 (53%) | 19 (7%) | 19 (7%) | 0 (0%) | 84 (32%) | 0 (0%) |
| ('desc_refplan', 'Qwen/Qwen3.8-27B@reasoning-none') | 1626 | 1395 (86%) | 59 (4%) | 121 (7%) | 6 (0%) | 18 (1%) | 27 (2%) |
| ('desc_refplan', 'deepseek-ai/DeepSeek-V4-Flash') | 387 | 176 (45%) | 83 (21%) | 84 (22%) | 2 (1%) | 33 (9%) | 9 (2%) |
| ('desc_refplan', 'moonshotai/Kimi-K3') | 13 | 9 (69%) | 1 (8%) | 0 (0%) | 0 (0%) | 2 (15%) | 1 (8%) |
| ('desc_refplan', 'zai-org/GLM-5.3-Flash') | 197 | 137 (70%) | 6 (3%) | 0 (0%) | 1 (1%) | 50 (25%) | 3 (2%) |

## Main category by run and level

| run and level | n | A | B | C | D | E | X |
|---|---|---|---|---|---|---|---|
| ('desc_full', 'L0') | 456 | 278 (61%) | 55 (12%) | 73 (16%) | 8 (2%) | 37 (8%) | 5 (1%) |
| ('desc_full', 'L1') | 482 | 313 (65%) | 54 (11%) | 63 (13%) | 5 (1%) | 44 (9%) | 3 (1%) |
| ('desc_full', 'L2') | 493 | 306 (62%) | 77 (16%) | 57 (12%) | 6 (1%) | 46 (9%) | 1 (0%) |
| ('desc_full', 'L3') | 461 | 286 (62%) | 67 (15%) | 60 (13%) | 3 (1%) | 42 (9%) | 3 (1%) |
| ('desc_plan', 'L0') | 542 | 441 (81%) | 42 (8%) | 20 (4%) | 7 (1%) | 29 (5%) | 3 (1%) |
| ('desc_plan', 'L1') | 603 | 466 (77%) | 55 (9%) | 22 (4%) | 5 (1%) | 47 (8%) | 8 (1%) |
| ('desc_plan', 'L2') | 618 | 492 (80%) | 45 (7%) | 28 (5%) | 7 (1%) | 44 (7%) | 2 (0%) |
| ('desc_plan', 'L3') | 561 | 423 (75%) | 50 (9%) | 24 (4%) | 9 (2%) | 47 (8%) | 8 (1%) |
| ('desc_refplan', 'L0') | 556 | 408 (73%) | 43 (8%) | 52 (9%) | 5 (1%) | 32 (6%) | 16 (3%) |
| ('desc_refplan', 'L1') | 538 | 421 (78%) | 33 (6%) | 55 (10%) | 2 (0%) | 22 (4%) | 5 (1%) |
| ('desc_refplan', 'L2') | 561 | 430 (77%) | 35 (6%) | 53 (9%) | 0 (0%) | 29 (5%) | 14 (2%) |
| ('desc_refplan', 'L3') | 568 | 458 (81%) | 38 (7%) | 45 (8%) | 2 (0%) | 20 (4%) | 5 (1%) |

## Main category by run and kernel category

| run and kernel category | n | A | B | C | D | E | X |
|---|---|---|---|---|---|---|---|
| ('desc_full', 'blas1') | 210 | 116 (55%) | 7 (3%) | 15 (7%) | 5 (2%) | 64 (30%) | 3 (1%) |
| ('desc_full', 'blas2') | 468 | 318 (68%) | 38 (8%) | 45 (10%) | 2 (0%) | 62 (13%) | 3 (1%) |
| ('desc_full', 'blas3') | 524 | 344 (66%) | 28 (5%) | 109 (21%) | 12 (2%) | 28 (5%) | 3 (1%) |
| ('desc_full', 'integer') | 144 | 95 (66%) | 21 (15%) | 23 (16%) | 2 (1%) | 3 (2%) | 0 (0%) |
| ('desc_full', 'irregular') | 172 | 124 (72%) | 6 (3%) | 39 (23%) | 0 (0%) | 1 (1%) | 2 (1%) |
| ('desc_full', 'stencil') | 374 | 186 (50%) | 153 (41%) | 22 (6%) | 1 (0%) | 11 (3%) | 1 (0%) |
| ('desc_plan', 'blas1') | 316 | 246 (78%) | 9 (3%) | 6 (2%) | 3 (1%) | 51 (16%) | 1 (0%) |
| ('desc_plan', 'blas2') | 670 | 521 (78%) | 36 (5%) | 34 (5%) | 11 (2%) | 61 (9%) | 7 (1%) |
| ('desc_plan', 'blas3') | 606 | 474 (78%) | 51 (8%) | 30 (5%) | 11 (2%) | 30 (5%) | 10 (2%) |
| ('desc_plan', 'integer') | 197 | 158 (80%) | 18 (9%) | 4 (2%) | 1 (1%) | 14 (7%) | 2 (1%) |
| ('desc_plan', 'irregular') | 194 | 167 (86%) | 12 (6%) | 13 (7%) | 0 (0%) | 1 (1%) | 1 (1%) |
| ('desc_plan', 'stencil') | 341 | 256 (75%) | 66 (19%) | 7 (2%) | 2 (1%) | 10 (3%) | 0 (0%) |
| ('desc_refplan', 'blas1') | 323 | 283 (88%) | 6 (2%) | 9 (3%) | 0 (0%) | 20 (6%) | 5 (2%) |
| ('desc_refplan', 'blas2') | 783 | 604 (77%) | 29 (4%) | 84 (11%) | 1 (0%) | 57 (7%) | 8 (1%) |
| ('desc_refplan', 'blas3') | 519 | 404 (78%) | 45 (9%) | 34 (7%) | 5 (1%) | 17 (3%) | 14 (3%) |
| ('desc_refplan', 'integer') | 75 | 64 (85%) | 1 (1%) | 9 (12%) | 0 (0%) | 0 (0%) | 1 (1%) |
| ('desc_refplan', 'irregular') | 163 | 110 (67%) | 8 (5%) | 45 (28%) | 0 (0%) | 0 (0%) | 0 (0%) |
| ('desc_refplan', 'stencil') | 360 | 252 (70%) | 60 (17%) | 24 (7%) | 3 (1%) | 9 (2%) | 12 (3%) |

## Main category by source

| source | n | A | B | C | D | E | X |
|---|---|---|---|---|---|---|---|
| rule | 6439 | 4722 (73%) | 594 (9%) | 552 (9%) | 59 (1%) | 439 (7%) | 73 (1%) |

## Assembler errors (A) by subtype, run and model

|  | n | directive | intel_syntax | invalid_operand | operand_order | other_syntax | preprocessor | size_suffix | unknown_mnemonic |
|---|---|---|---|---|---|---|---|---|---|
| ('desc_full', 'Qwen/Qwen3.8-27B@reasoning-none') | 791 | 17 (2%) | 134 (17%) | 470 (59%) | 6 (1%) | 13 (2%) | 61 (8%) | 75 (9%) | 15 (2%) |
| ('desc_full', 'deepseek-ai/DeepSeek-V4-Flash') | 254 | 1 (0%) | 15 (6%) | 198 (78%) | 1 (0%) | 6 (2%) | 28 (11%) | 1 (0%) | 4 (2%) |
| ('desc_full', 'moonshotai/Kimi-K3') | 20 | 2 (10%) | 1 (5%) | 12 (60%) | 0 (0%) | 0 (0%) | 1 (5%) | 4 (20%) | 0 (0%) |
| ('desc_full', 'zai-org/GLM-5.3-Flash') | 118 | 0 (0%) | 39 (33%) | 57 (48%) | 1 (1%) | 7 (6%) | 10 (8%) | 1 (1%) | 3 (3%) |
| ('desc_plan', 'Qwen/Qwen3.8-27B@reasoning-none') | 1381 | 32 (2%) | 116 (8%) | 975 (71%) | 13 (1%) | 26 (2%) | 70 (5%) | 51 (4%) | 98 (7%) |
| ('desc_plan', 'deepseek-ai/DeepSeek-V4-Flash') | 286 | 2 (1%) | 5 (2%) | 219 (77%) | 2 (1%) | 11 (4%) | 38 (13%) | 5 (2%) | 4 (1%) |
| ('desc_plan', 'moonshotai/Kimi-K3') | 16 | 0 (0%) | 2 (12%) | 8 (50%) | 0 (0%) | 3 (19%) | 3 (19%) | 0 (0%) | 0 (0%) |
| ('desc_plan', 'zai-org/GLM-5.3-Flash') | 139 | 4 (3%) | 48 (35%) | 65 (47%) | 0 (0%) | 4 (3%) | 11 (8%) | 1 (1%) | 6 (4%) |
| ('desc_refplan', 'Qwen/Qwen3.8-27B@reasoning-none') | 1395 | 23 (2%) | 424 (30%) | 735 (53%) | 5 (0%) | 13 (1%) | 72 (5%) | 45 (3%) | 78 (6%) |
| ('desc_refplan', 'deepseek-ai/DeepSeek-V4-Flash') | 176 | 2 (1%) | 10 (6%) | 125 (71%) | 0 (0%) | 3 (2%) | 29 (16%) | 4 (2%) | 3 (2%) |
| ('desc_refplan', 'moonshotai/Kimi-K3') | 9 | 0 (0%) | 1 (11%) | 1 (11%) | 0 (0%) | 2 (22%) | 1 (11%) | 4 (44%) | 0 (0%) |
| ('desc_refplan', 'zai-org/GLM-5.3-Flash') | 137 | 1 (1%) | 79 (58%) | 39 (28%) | 1 (1%) | 3 (2%) | 9 (7%) | 2 (1%) | 3 (2%) |

## C by subtype and run

|  | n | callee_saved | stack_alignment | stack_pointer |
|---|---|---|---|---|
| desc_full | 253 | 215 (85%) | 17 (7%) | 21 (8%) |
| desc_plan | 94 | 75 (80%) | 9 (10%) | 10 (11%) |
| desc_refplan | 205 | 194 (95%) | 0 (0%) | 11 (5%) |

## D by subtype and run

|  | n | aligned_access |
|---|---|---|
| desc_full | 22 | 22 (100%) |
| desc_plan | 28 | 28 (100%) |
| desc_refplan | 9 | 9 (100%) |

## E by subtype and run

|  | n | small_n | tail |
|---|---|---|---|
| desc_full | 169 | 39 (23%) | 130 (77%) |
| desc_plan | 167 | 30 (18%) | 137 (82%) |
| desc_refplan | 103 | 30 (29%) | 73 (71%) |

## F by subtype and run

|  | n |
|---|---|

## Repair transitions: category at attempt k (rows) -> attempt k+1 (columns), weighted cells

| from \ to | n | A | B | C | D | E | X | pass | fixed | same category |
|---|---|---|---|---|---|---|---|---|---|---|
| A | 3176 | 2343 | 151 | 152 | 23 | 70 | 13 | 424 | 424 (13%) | 2343 (74%) |
| B | 382 | 99 | 171 | 27 | 5 | 12 | 0 | 68 | 68 (18%) | 171 (45%) |
| C | 320 | 36 | 12 | 137 | 1 | 4 | 0 | 131 | 131 (41%) | 137 (43%) |
| D | 30 | 4 | 2 | 0 | 18 | 1 | 0 | 5 | 5 (17%) | 18 (60%) |
| E | 345 | 11 | 1 | 7 | 0 | 113 | 0 | 214 | 214 (62%) | 113 (33%) |
| X | 51 | 36 | 2 | 4 | 0 | 0 | 3 | 7 | 7 (13%) | 3 (6%) |

## desc_full vs desc_plan vs desc_refplan on matched cells (same model, kernel, level, repeat)

1920 matched cells. Failed attempts per 100 cells (all attempts / first attempt only); C-G are partly estimated from the judged sample.

| category | desc_full | desc_plan | desc_refplan | refplan - full |
|---|---|---|---|---|
| A | 61.6 / 27.9 | 94.9 / 34.2 | 89.4 / 31.3 | 27.8 / 3.4 |
| B | 13.2 / 4.6 | 10.0 / 3.2 | 7.8 / 2.7 | -5.4 / -1.9 |
| C | 13.2 / 4.2 | 4.9 / 1.6 | 10.7 / 2.4 | -2.5 / -1.7 |
| D | 1.1 / 0.2 | 1.5 / 0.1 | 0.5 / 0.1 | -0.7 / -0.1 |
| E | 8.8 / 4.4 | 8.7 / 3.1 | 5.4 / 1.9 | -3.4 / -2.5 |
| X | 0.6 / 0.4 | 1.1 / 0.5 | 2.1 / 0.9 | 1.5 / 0.6 |

