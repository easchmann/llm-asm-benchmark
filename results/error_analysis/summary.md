# Failure classification

One main category per failed attempt; see prompts/error_rubric.md. A, B, X and the rule-based C/D/E are exact counts. Test failures the rules could not classify were judged for a random sample of whole cells (judge_sample.tsv) and weighted up to their run, so the remaining C-G counts are estimates. Cells: count (share of the row's failed attempts).

## Main category by run

| run | n | A | B | C | D | E | F | X |
|---|---|---|---|---|---|---|---|---|
| desc_full | 3238 | 1183 (37%) | 253 (8%) | 630 (19%) | 620 (19%) | 169 (5%) | 371 (11%) | 12 (0%) |
| desc_plan | 3786 | 1822 (48%) | 192 (5%) | 381 (10%) | 864 (23%) | 167 (4%) | 339 (9%) | 21 (1%) |
| desc_refplan | 3133 | 1717 (55%) | 149 (5%) | 288 (9%) | 647 (21%) | 103 (3%) | 189 (6%) | 40 (1%) |

## Judged categories within the sample of pending attempts (share, 95% interval)

Pending attempts are the test failures the rules could not classify. Intervals treat attempts as independent; attempts of one cell are correlated, so they are somewhat too narrow.

| run | judged | C | D | E | F | G |
|---|---|---|---|---|---|---|
| desc_full | 250 | 28% (22-34) | 44% (38-51) | 0% (0-0) | 28% (22-33) | 0% (0-0) |
| desc_plan | 250 | 20% (15-25) | 57% (51-63) | 0% (0-0) | 23% (18-28) | 0% (0-0) |
| desc_refplan | 251 | 9% (6-13) | 70% (64-76) | 0% (0-0) | 21% (16-26) | 0% (0-0) |

## Main category by run and model

| run and model | n | A | B | C | D | E | F | X |
|---|---|---|---|---|---|---|---|---|
| ('desc_full', 'Qwen/Qwen3.8-27B@reasoning-none') | 1434 | 791 (55%) | 61 (4%) | 308 (21%) | 188 (13%) | 12 (1%) | 65 (5%) | 9 (1%) |
| ('desc_full', 'deepseek-ai/DeepSeek-V4-Flash') | 1136 | 254 (22%) | 178 (16%) | 239 (21%) | 273 (24%) | 51 (4%) | 140 (12%) | 1 (0%) |
| ('desc_full', 'moonshotai/Kimi-K3') | 66 | 20 (30%) | 1 (2%) | 6 (10%) | 11 (16%) | 28 (42%) | 0 (0%) | 0 (0%) |
| ('desc_full', 'zai-org/GLM-5.3-Flash') | 602 | 118 (20%) | 13 (2%) | 76 (13%) | 147 (24%) | 78 (13%) | 167 (28%) | 2 (0%) |
| ('desc_plan', 'Qwen/Qwen3.8-27B@reasoning-none') | 2003 | 1381 (69%) | 99 (5%) | 171 (9%) | 285 (14%) | 14 (1%) | 35 (2%) | 18 (1%) |
| ('desc_plan', 'deepseek-ai/DeepSeek-V4-Flash') | 1072 | 286 (27%) | 74 (7%) | 161 (15%) | 322 (30%) | 64 (6%) | 164 (15%) | 2 (0%) |
| ('desc_plan', 'moonshotai/Kimi-K3') | 64 | 16 (25%) | 0 (0%) | 7 (11%) | 35 (55%) | 5 (8%) | 0 (0%) | 1 (2%) |
| ('desc_plan', 'zai-org/GLM-5.3-Flash') | 647 | 139 (21%) | 19 (3%) | 42 (7%) | 222 (34%) | 84 (13%) | 140 (22%) | 0 (0%) |
| ('desc_refplan', 'Qwen/Qwen3.8-27B@reasoning-none') | 2032 | 1395 (69%) | 59 (3%) | 165 (8%) | 311 (15%) | 18 (1%) | 58 (3%) | 27 (1%) |
| ('desc_refplan', 'deepseek-ai/DeepSeek-V4-Flash') | 746 | 176 (24%) | 83 (11%) | 120 (16%) | 263 (35%) | 33 (4%) | 62 (8%) | 9 (1%) |
| ('desc_refplan', 'moonshotai/Kimi-K3') | 17 | 9 (54%) | 1 (6%) | 0 (0%) | 4 (22%) | 2 (12%) | 0 (0%) | 1 (6%) |
| ('desc_refplan', 'zai-org/GLM-5.3-Flash') | 338 | 137 (40%) | 6 (2%) | 4 (1%) | 70 (21%) | 50 (15%) | 69 (20%) | 3 (1%) |

## Main category by run and level

| run and level | n | A | B | C | D | E | F | X |
|---|---|---|---|---|---|---|---|---|
| ('desc_full', 'L0') | 763 | 278 (36%) | 55 (7%) | 202 (27%) | 116 (15%) | 37 (5%) | 70 (9%) | 5 (1%) |
| ('desc_full', 'L1') | 757 | 313 (41%) | 54 (7%) | 95 (13%) | 150 (20%) | 44 (6%) | 97 (13%) | 3 (0%) |
| ('desc_full', 'L2') | 821 | 306 (37%) | 77 (9%) | 127 (15%) | 184 (22%) | 46 (6%) | 81 (10%) | 1 (0%) |
| ('desc_full', 'L3') | 897 | 286 (32%) | 67 (7%) | 205 (23%) | 170 (19%) | 42 (5%) | 124 (14%) | 3 (0%) |
| ('desc_plan', 'L0') | 852 | 441 (52%) | 42 (5%) | 73 (9%) | 206 (24%) | 29 (3%) | 58 (7%) | 3 (0%) |
| ('desc_plan', 'L1') | 1053 | 466 (44%) | 55 (5%) | 133 (13%) | 251 (24%) | 47 (4%) | 94 (9%) | 8 (1%) |
| ('desc_plan', 'L2') | 975 | 492 (50%) | 45 (5%) | 69 (7%) | 229 (24%) | 44 (5%) | 94 (10%) | 2 (0%) |
| ('desc_plan', 'L3') | 906 | 423 (47%) | 50 (6%) | 106 (12%) | 179 (20%) | 47 (5%) | 94 (10%) | 8 (1%) |
| ('desc_refplan', 'L0') | 745 | 408 (55%) | 43 (6%) | 59 (8%) | 143 (19%) | 32 (4%) | 44 (6%) | 16 (2%) |
| ('desc_refplan', 'L1') | 756 | 421 (56%) | 33 (4%) | 88 (12%) | 136 (18%) | 22 (3%) | 51 (7%) | 5 (1%) |
| ('desc_refplan', 'L2') | 829 | 430 (52%) | 35 (4%) | 68 (8%) | 203 (24%) | 29 (3%) | 51 (6%) | 14 (2%) |
| ('desc_refplan', 'L3') | 804 | 458 (57%) | 38 (5%) | 74 (9%) | 165 (21%) | 20 (2%) | 44 (5%) | 5 (1%) |

## Main category by run and kernel category

| run and kernel category | n | A | B | C | D | E | F | X |
|---|---|---|---|---|---|---|---|---|
| ('desc_full', 'blas1') | 345 | 116 (34%) | 7 (2%) | 42 (12%) | 80 (23%) | 64 (19%) | 32 (9%) | 3 (1%) |
| ('desc_full', 'blas2') | 915 | 318 (35%) | 38 (4%) | 190 (21%) | 147 (16%) | 62 (7%) | 156 (17%) | 3 (0%) |
| ('desc_full', 'blas3') | 874 | 344 (39%) | 28 (3%) | 244 (28%) | 157 (18%) | 28 (3%) | 70 (8%) | 3 (0%) |
| ('desc_full', 'integer') | 225 | 95 (42%) | 21 (9%) | 23 (10%) | 50 (22%) | 3 (1%) | 32 (14%) | 0 (0%) |
| ('desc_full', 'irregular') | 317 | 124 (39%) | 6 (2%) | 87 (28%) | 92 (29%) | 1 (0%) | 5 (2%) | 2 (1%) |
| ('desc_full', 'stencil') | 562 | 186 (33%) | 153 (27%) | 44 (8%) | 93 (16%) | 11 (2%) | 75 (13%) | 1 (0%) |
| ('desc_plan', 'blas1') | 509 | 246 (48%) | 9 (2%) | 70 (14%) | 91 (18%) | 51 (10%) | 41 (8%) | 1 (0%) |
| ('desc_plan', 'blas2') | 1056 | 521 (49%) | 36 (3%) | 145 (14%) | 210 (20%) | 61 (6%) | 76 (7%) | 7 (1%) |
| ('desc_plan', 'blas3') | 963 | 474 (49%) | 51 (5%) | 112 (12%) | 227 (24%) | 30 (3%) | 58 (6%) | 10 (1%) |
| ('desc_plan', 'integer') | 326 | 158 (49%) | 18 (6%) | 4 (1%) | 77 (24%) | 14 (4%) | 53 (16%) | 2 (1%) |
| ('desc_plan', 'irregular') | 405 | 167 (41%) | 12 (3%) | 42 (10%) | 170 (42%) | 1 (0%) | 12 (3%) | 1 (0%) |
| ('desc_plan', 'stencil') | 528 | 256 (48%) | 66 (12%) | 7 (1%) | 90 (17%) | 10 (2%) | 99 (19%) | 0 (0%) |
| ('desc_refplan', 'blas1') | 377 | 283 (75%) | 6 (2%) | 9 (2%) | 36 (10%) | 20 (5%) | 18 (5%) | 5 (1%) |
| ('desc_refplan', 'blas2') | 1084 | 604 (56%) | 29 (3%) | 95 (9%) | 237 (22%) | 57 (5%) | 54 (5%) | 8 (1%) |
| ('desc_refplan', 'blas3') | 809 | 404 (50%) | 45 (6%) | 96 (12%) | 161 (20%) | 17 (2%) | 73 (9%) | 14 (2%) |
| ('desc_refplan', 'integer') | 144 | 64 (44%) | 1 (1%) | 9 (6%) | 65 (45%) | 0 (0%) | 4 (3%) | 1 (1%) |
| ('desc_refplan', 'irregular') | 239 | 110 (46%) | 8 (3%) | 56 (23%) | 54 (23%) | 0 (0%) | 11 (5%) | 0 (0%) |
| ('desc_refplan', 'stencil') | 480 | 252 (53%) | 60 (13%) | 24 (5%) | 94 (20%) | 9 (2%) | 29 (6%) | 12 (3%) |

## Main category by source

| source | n | A | B | C | D | E | F | X |
|---|---|---|---|---|---|---|---|---|
| judge | 3718 | 0 (0%) | 0 (0%) | 747 (20%) | 2072 (56%) | 0 (0%) | 899 (24%) | 0 (0%) |
| rule | 6439 | 4722 (73%) | 594 (9%) | 552 (9%) | 59 (1%) | 439 (7%) | 0 (0%) | 73 (1%) |

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

|  | n | arg_registers | callee_saved | stack_alignment | stack_pointer |
|---|---|---|---|---|---|
| desc_full | 630 | 345 (55%) | 215 (34%) | 17 (3%) | 53 (8%) |
| desc_plan | 381 | 281 (74%) | 81 (21%) | 9 (2%) | 10 (3%) |
| desc_refplan | 288 | 73 (25%) | 198 (69%) | 0 (0%) | 18 (6%) |

## D by subtype and run

|  | n | addressing | aligned_access | element_size | flags_condition | horizontal_reduction | isa | lane_order | operand_order | other_semantics | packed_scalar | reg_conflict | shuffle_permute_blend |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| desc_full | 620 | 32 (5%) | 22 (4%) | 75 (12%) | 54 (9%) | 70 (11%) | 70 (11%) | 5 (1%) | 27 (4%) | 11 (2%) | 32 (5%) | 178 (29%) | 43 (7%) |
| desc_plan | 864 | 70 (8%) | 28 (3%) | 29 (3%) | 29 (3%) | 88 (10%) | 129 (15%) | 0 (0%) | 88 (10%) | 35 (4%) | 41 (5%) | 292 (34%) | 35 (4%) |
| desc_refplan | 647 | 91 (14%) | 9 (1%) | 25 (4%) | 73 (11%) | 58 (9%) | 29 (4%) | 7 (1%) | 7 (1%) | 4 (1%) | 0 (0%) | 341 (53%) | 4 (1%) |

## E by subtype and run

|  | n | small_n | tail |
|---|---|---|---|
| desc_full | 169 | 39 (23%) | 130 (77%) |
| desc_plan | 167 | 30 (18%) | 137 (82%) |
| desc_refplan | 103 | 30 (29%) | 73 (71%) |

## F by subtype and run

|  | n | indexing | initialization | loop_bounds | math | other_algorithm | stride | update_order |
|---|---|---|---|---|---|---|---|---|
| desc_full | 371 | 162 (43%) | 16 (4%) | 124 (33%) | 43 (12%) | 16 (4%) | 5 (1%) | 5 (1%) |
| desc_plan | 339 | 140 (41%) | 29 (9%) | 105 (31%) | 23 (7%) | 23 (7%) | 12 (3%) | 6 (2%) |
| desc_refplan | 189 | 80 (42%) | 33 (17%) | 47 (25%) | 7 (4%) | 22 (12%) | 0 (0%) | 0 (0%) |

## Repair transitions: category at attempt k (rows) -> attempt k+1 (columns), weighted cells

| from \ to | n | A | B | C | D | E | F | X | pass | fixed | same category |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 3906 | 2343 | 151 | 354 | 408 | 70 | 143 | 13 | 424 | 424 (11%) | 2343 (60%) |
| B | 576 | 99 | 171 | 85 | 100 | 12 | 40 | 0 | 68 | 68 (12%) | 171 (30%) |
| C | 973 | 85 | 38 | 435 | 139 | 16 | 60 | 6 | 195 | 195 (20%) | 435 (45%) |
| D | 1475 | 123 | 31 | 75 | 724 | 41 | 90 | 13 | 378 | 378 (26%) | 724 (49%) |
| E | 365 | 11 | 1 | 7 | 4 | 113 | 16 | 0 | 214 | 214 (59%) | 113 (31%) |
| F | 689 | 51 | 21 | 26 | 54 | 42 | 178 | 4 | 314 | 314 (46%) | 178 (26%) |
| X | 67 | 36 | 2 | 10 | 10 | 0 | 0 | 3 | 7 | 7 (10%) | 3 (4%) |

## desc_full vs desc_plan vs desc_refplan on matched cells (same model, kernel, level, repeat)

1920 matched cells. Failed attempts per 100 cells (all attempts / first attempt only); C-G are partly estimated from the judged sample.

| category | desc_full | desc_plan | desc_refplan | refplan - full |
|---|---|---|---|---|
| A | 61.6 / 27.9 | 94.9 / 34.2 | 89.4 / 31.3 | 27.8 / 3.4 |
| B | 13.2 / 4.6 | 10.0 / 3.2 | 7.8 / 2.7 | -5.4 / -1.9 |
| C | 32.8 / 9.2 | 19.8 / 4.4 | 15.0 / 3.2 | -17.8 / -6.0 |
| D | 32.3 / 11.7 | 45.0 / 15.0 | 33.7 / 8.6 | 1.4 / -3.1 |
| E | 8.8 / 4.4 | 8.7 / 3.1 | 5.4 / 1.9 | -3.4 / -2.5 |
| F | 19.3 / 9.5 | 17.7 / 6.1 | 9.8 / 3.4 | -9.5 / -6.1 |
| X | 0.6 / 0.4 | 1.1 / 0.5 | 2.1 / 0.9 | 1.5 / 0.6 |

