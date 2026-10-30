## Per model

| model | cells | no answer | pass first try | pass after repair | cost |
|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 480 | 0 | 9% | 34% | $0.80 |
| deepseek-ai/DeepSeek-V4-Flash | 480 | 0 | 20% | 52% | $3.66 |
| moonshotai/Kimi-K3 | 480 | 0 | 86% | 99% | $44.92 |
| zai-org/GLM-5.3-Flash | 480 | 0 | 32% | 83% | $2.95 |

## Per model and level

| model | level | cells | no answer | pass first try | pass after repair | cost |
|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | L0 | 120 | 0 | 11% | 42% | $0.19 |
| Qwen/Qwen3.8-27B@reasoning-none | L1 | 120 | 0 | 10% | 32% | $0.20 |
| Qwen/Qwen3.8-27B@reasoning-none | L2 | 120 | 0 | 10% | 30% | $0.21 |
| Qwen/Qwen3.8-27B@reasoning-none | L3 | 120 | 0 | 4% | 31% | $0.21 |
| deepseek-ai/DeepSeek-V4-Flash | L0 | 120 | 0 | 24% | 60% | $0.77 |
| deepseek-ai/DeepSeek-V4-Flash | L1 | 120 | 0 | 18% | 52% | $0.94 |
| deepseek-ai/DeepSeek-V4-Flash | L2 | 120 | 0 | 16% | 43% | $1.02 |
| deepseek-ai/DeepSeek-V4-Flash | L3 | 120 | 0 | 22% | 52% | $0.93 |
| moonshotai/Kimi-K3 | L0 | 120 | 0 | 82% | 100% | $11.48 |
| moonshotai/Kimi-K3 | L1 | 120 | 0 | 87% | 100% | $11.28 |
| moonshotai/Kimi-K3 | L2 | 120 | 0 | 90% | 98% | $10.77 |
| moonshotai/Kimi-K3 | L3 | 120 | 0 | 86% | 98% | $11.39 |
| zai-org/GLM-5.3-Flash | L0 | 120 | 0 | 29% | 82% | $0.68 |
| zai-org/GLM-5.3-Flash | L1 | 120 | 0 | 29% | 85% | $0.66 |
| zai-org/GLM-5.3-Flash | L2 | 120 | 0 | 30% | 80% | $0.78 |
| zai-org/GLM-5.3-Flash | L3 | 120 | 0 | 39% | 86% | $0.83 |

## Per model and category

| model | category | cells | no answer | pass first try | pass after repair | cost |
|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | blas1 | 80 | 0 | 20% | 60% | $0.05 |
| Qwen/Qwen3.8-27B@reasoning-none | blas2 | 120 | 0 | 3% | 31% | $0.24 |
| Qwen/Qwen3.8-27B@reasoning-none | blas3 | 100 | 0 | 1% | 15% | $0.25 |
| Qwen/Qwen3.8-27B@reasoning-none | integer | 60 | 0 | 23% | 63% | $0.03 |
| Qwen/Qwen3.8-27B@reasoning-none | irregular | 60 | 0 | 7% | 23% | $0.08 |
| Qwen/Qwen3.8-27B@reasoning-none | stencil | 60 | 0 | 5% | 18% | $0.15 |
| deepseek-ai/DeepSeek-V4-Flash | blas1 | 80 | 0 | 38% | 68% | $0.24 |
| deepseek-ai/DeepSeek-V4-Flash | blas2 | 120 | 0 | 18% | 52% | $0.82 |
| deepseek-ai/DeepSeek-V4-Flash | blas3 | 100 | 0 | 7% | 40% | $1.07 |
| deepseek-ai/DeepSeek-V4-Flash | integer | 60 | 0 | 32% | 72% | $0.22 |
| deepseek-ai/DeepSeek-V4-Flash | irregular | 60 | 0 | 27% | 50% | $0.29 |
| deepseek-ai/DeepSeek-V4-Flash | stencil | 60 | 0 | 3% | 33% | $1.02 |
| moonshotai/Kimi-K3 | blas1 | 80 | 0 | 84% | 96% | $3.80 |
| moonshotai/Kimi-K3 | blas2 | 120 | 0 | 86% | 99% | $11.89 |
| moonshotai/Kimi-K3 | blas3 | 100 | 0 | 82% | 100% | $13.11 |
| moonshotai/Kimi-K3 | integer | 60 | 0 | 92% | 100% | $3.77 |
| moonshotai/Kimi-K3 | irregular | 60 | 0 | 87% | 98% | $5.15 |
| moonshotai/Kimi-K3 | stencil | 60 | 0 | 92% | 100% | $7.19 |
| zai-org/GLM-5.3-Flash | blas1 | 80 | 0 | 44% | 89% | $0.14 |
| zai-org/GLM-5.3-Flash | blas2 | 120 | 0 | 22% | 78% | $0.85 |
| zai-org/GLM-5.3-Flash | blas3 | 100 | 0 | 26% | 83% | $0.79 |
| zai-org/GLM-5.3-Flash | integer | 60 | 0 | 43% | 78% | $0.19 |
| zai-org/GLM-5.3-Flash | irregular | 60 | 0 | 38% | 87% | $0.51 |
| zai-org/GLM-5.3-Flash | stencil | 60 | 0 | 27% | 90% | $0.48 |

## Final stage of cells that did not pass

| model | extract_fail | incomplete | assemble_fail | link_fail | crash | timeout | wrong |
|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 0 | 1 | 119 | 2 | 99 | 1 | 95 |
| deepseek-ai/DeepSeek-V4-Flash | 0 | 0 | 37 | 0 | 79 | 0 | 115 |
| moonshotai/Kimi-K3 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| zai-org/GLM-5.3-Flash | 0 | 0 | 2 | 0 | 14 | 0 | 62 |

## Pass rate (95% CI, bootstrap over kernels)

| model | cells | first try | after repair |
|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 480 | 9% [5-13] | 34% [24-44] |
| DeepSeek-V4-Flash | 480 | 20% [14-26] | 52% [43-60] |
| Kimi-K3 | 480 | 86% [84-89] | 99% [98-100] |
| GLM-5.3-Flash | 480 | 32% [26-39] | 83% [79-88] |

## Level effect: L3 minus L0, paired by kernel (95% CI, bootstrap over kernels)

| model |  | L3 - L0 | kernels better | same | worse |
|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | first try | -7 pts [-12, -1] | 2 | 13 | 9 |
| Qwen3.8-27B@reasoning-none | after repair | -12 pts [-22, -0] | 5 | 5 | 14 |
| DeepSeek-V4-Flash | first try | -2 pts [-13, +9] | 7 | 10 | 7 |
| DeepSeek-V4-Flash | after repair | -8 pts [-17, +3] | 6 | 6 | 12 |
| Kimi-K3 | first try | +3 pts [-6, +12] | 10 | 5 | 9 |
| Kimi-K3 | after repair | -2 pts [-4, +0] | 0 | 22 | 2 |
| GLM-5.3-Flash | first try | +10 pts [+2, +17] | 12 | 10 | 2 |
| GLM-5.3-Flash | after repair | +3 pts [-5, +12] | 10 | 7 | 7 |

## Pass rate by level

| model | L0 first / repaired | L1 first / repaired | L2 first / repaired | L3 first / repaired |
|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 11% / 42% | 10% / 32% | 10% / 30% | 4% / 31% |
| DeepSeek-V4-Flash | 24% / 60% | 18% / 52% | 16% / 43% | 22% / 52% |
| Kimi-K3 | 82% / 100% | 87% / 100% | 90% / 98% | 86% / 98% |
| GLM-5.3-Flash | 29% / 82% | 29% / 85% | 30% / 80% | 39% / 86% |

## Why first tries fail (share of failing first tries)

| model | failing | assemble_fail | wrong: small sizes pass | crash SIGSEGV | wrong: all sizes | wrong: ABI | crash SIGABRT | crash SIGILL | wrong: out-of-bounds write | link_fail | wrong: other | incomplete | timeout | crash SIGBUS | wrong: input modified | extract_fail | crash SIGFPE |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 438 | 68% | 3% | 11% | 7% | 4% | 1% | 1% | 1% | 1% | 1% | 1% | 0% | 1% | 0% | 0% | 0% |
| DeepSeek-V4-Flash | 384 | 48% | 9% | 16% | 5% | 8% | 2% | 5% | 2% | 4% | 1% | 0% | 0% | 0% | 0% | 0% | 0% |
| Kimi-K3 | 66 | 30% | 45% | 5% | 8% | 2% | 2% | 0% | 3% | 2% | 3% | 0% | 2% | 0% | 0% | 0% | 0% |
| GLM-5.3-Flash | 327 | 30% | 28% | 12% | 13% | 4% | 6% | 1% | 3% | 1% | 2% | 0% | 0% | 0% | 0% | 0% | 0% |

## Kernels (first try / after repair)

| kernel | source | Qwen3.8-27B@reasoning-none | DeepSeek-V4-Flash | Kimi-K3 | GLM-5.3-Flash |
|---|---|---|---|---|---|
| 2mm | PolyBench | 0% / 10% | 0% / 30% | 90% / 100% | 10% / 80% |
| gemver | PolyBench | 0% / 5% | 0% / 15% | 90% / 100% | 15% / 90% |
| gesummv | PolyBench | 0% / 5% | 5% / 25% | 85% / 100% | 15% / 60% |
| heat3d | PolyBench | 0% / 5% | 0% / 15% | 90% / 100% | 15% / 85% |
| trmm | PolyBench | 0% / 25% | 10% / 45% | 85% / 100% | 15% / 80% |
| jacobi2d | PolyBench | 0% / 5% | 0% / 35% | 95% / 100% | 20% / 95% |
| 3mm | PolyBench | 0% / 0% | 0% / 10% | 80% / 100% | 35% / 90% |
| syrk | PolyBench | 5% / 15% | 15% / 50% | 75% / 100% | 20% / 70% |
| bicg | PolyBench | 0% / 20% | 15% / 60% | 85% / 100% | 20% / 85% |
| mvt | PolyBench | 10% / 50% | 20% / 70% | 75% / 100% | 20% / 85% |
| sort8 | other | 5% / 15% | 10% / 30% | 90% / 100% | 35% / 85% |
| gemm | PolyBench | 0% / 25% | 10% / 65% | 80% / 100% | 50% / 95% |
| atax | PolyBench | 0% / 50% | 20% / 60% | 90% / 95% | 35% / 70% |
| lower_bound | other | 5% / 10% | 35% / 50% | 85% / 100% | 30% / 85% |
| jacobi1d | PolyBench | 15% / 45% | 10% / 50% | 90% / 100% | 45% / 90% |
| dot | other | 5% / 50% | 45% / 55% | 85% / 95% | 30% / 80% |
| prefix_sum | other | 25% / 75% | 30% / 70% | 80% / 100% | 35% / 50% |
| nrm2 | other | 35% / 70% | 35% / 75% | 75% / 90% | 35% / 85% |
| trmv | other | 10% / 55% | 50% / 80% | 90% / 100% | 30% / 75% |
| popcount | other | 25% / 55% | 40% / 70% | 95% / 100% | 20% / 85% |
| spmv | other | 10% / 45% | 35% / 70% | 85% / 95% | 50% / 90% |
| scal | other | 20% / 75% | 35% / 65% | 80% / 100% | 55% / 90% |
| axpy | other | 20% / 45% | 35% / 75% | 95% / 100% | 55% / 100% |
| histogram | other | 20% / 60% | 25% / 75% | 100% / 100% | 75% / 100% |

## PolyBench-derived vs other kernels (first try / after repair)

| model | PolyBench | other |
|---|---|---|
| Qwen3.8-27B@reasoning-none | 2% / 20% (n=260) | 16% / 50% (n=220) |
| DeepSeek-V4-Flash | 8% / 41% (n=260) | 34% / 65% (n=220) |
| Kimi-K3 | 85% / 100% (n=260) | 87% / 98% (n=220) |
| GLM-5.3-Flash | 24% / 83% (n=260) | 41% / 84% (n=220) |

## Most common assembler errors (first try)

| count | model | error |
|---|---|---|
| 31 | Qwen3.8-27B@reasoning-none | operand size mismatch for `mov' |
| 30 | DeepSeek-V4-Flash | operand type mismatch for `vmovsd' |
| 23 | GLM-5.3-Flash | operand size mismatch for `mov' |
| 23 | Qwen3.8-27B@reasoning-none | incorrect register `X` used with `X` suffix |
| 18 | DeepSeek-V4-Flash | bad register name `X` |
| 18 | Qwen3.8-27B@reasoning-none | operand type mismatch for `movsd' |
| 18 | Qwen3.8-27B@reasoning-none | bad register name `X` |
| 16 | Qwen3.8-27B@reasoning-none | junk `X` after expression |
| 12 | Qwen3.8-27B@reasoning-none | symbol `X` is already defined |
| 12 | Qwen3.8-27B@reasoning-none | number of operands mismatch for `mul' |
| 11 | DeepSeek-V4-Flash | expecting scale factor of 1, 2, 4, or 8: got `X` |
| 10 | DeepSeek-V4-Flash | symbol `X` is already defined |
| 9 | DeepSeek-V4-Flash | operand type mismatch for `mov' |
| 9 | Qwen3.8-27B@reasoning-none | expecting scale factor of 1, 2, 4, or 8: got `X` |
| 9 | Qwen3.8-27B@reasoning-none | operand size mismatch for `addsd' |

## Checks on passing code (final round)

| model | passing cells | with call instruction | immediate equal to a test size (check by hand) |
|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 163 | 0 | 3 |
| DeepSeek-V4-Flash | 249 | 1 | 4 |
| Kimi-K3 | 475 | 0 | 30 |
| GLM-5.3-Flash | 400 | 15 | 36 |

## Cumulative pass rate by round (cells with an answer in round 0)

| model | cells | r0 | r1 | r2 | r3 | r4 | r5 |
|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 480 | 9% | 21% | 28% | 31% | 33% | 34% |
| deepseek-ai/DeepSeek-V4-Flash | 480 | 20% | 36% | 44% | 47% | 51% | 52% |
| moonshotai/Kimi-K3 | 480 | 86% | 98% | 99% | 99% | 99% | 99% |
| zai-org/GLM-5.3-Flash | 480 | 32% | 64% | 77% | 82% | 83% | 83% |

## Trajectory classes

| model | first try | fixed | progressing | oscillating | stuck | regressing | interrupted | not repaired | no answer |
|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 42 | 121 | 92 | 73 | 145 | 7 | 0 | 0 | 0 |
| deepseek-ai/DeepSeek-V4-Flash | 96 | 153 | 85 | 31 | 109 | 6 | 0 | 0 | 0 |
| moonshotai/Kimi-K3 | 414 | 61 | 0 | 0 | 1 | 0 | 4 | 0 | 0 |
| zai-org/GLM-5.3-Flash | 153 | 247 | 17 | 12 | 47 | 2 | 2 | 0 | 0 |

## Stage transitions between consecutive rounds (all cells)

| from | to | count |
|---|---|---|
| assemble_fail | assemble_fail | 657 |
| wrong | wrong | 393 |
| wrong | pass | 305 |
| crash | crash | 223 |
| assemble_fail | wrong | 211 |
| assemble_fail | crash | 175 |
| assemble_fail | pass | 172 |
| crash | wrong | 103 |
| crash | pass | 98 |
| wrong | assemble_fail | 61 |
| crash | assemble_fail | 56 |
| wrong | crash | 49 |
| link_fail | crash | 16 |
| link_fail | wrong | 15 |
| assemble_fail | link_fail | 11 |
| link_fail | assemble_fail | 5 |
| wrong | link_fail | 5 |
| wrong | no_answer | 5 |
| incomplete | assemble_fail | 5 |
| link_fail | pass | 4 |

## Repair cost per model

| model | fixed | mean round | repair calls | repair cost | cost per fix |
|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 121 | 1.9 | 1162 | $0.561 | $0.0046 |
| deepseek-ai/DeepSeek-V4-Flash | 153 | 1.9 | 861 | $2.499 | $0.0163 |
| moonshotai/Kimi-K3 | 61 | 1.0 | 69 | $4.634 | $0.0760 |
| zai-org/GLM-5.3-Flash | 247 | 1.5 | 519 | $1.539 | $0.0062 |

## Stop reasons

| stop reason | cells |
|---|---|
| pass | 1287 |
| same_errors | 514 |
| budget | 113 |
| api_error | 6 |

## Consistency of the runs per model-task pair (first try)

| model | tasks | identical code | same stage | same pass/fail | stable (<= 1 differ) | pass rate (all runs) | majority pass | all runs pass |
|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none | 96 | 0% | 22% | 69% | 91% | 9% | 3% | 0% |
| deepseek-ai/DeepSeek-V4-Flash | 96 | 0% | 6% | 43% | 73% | 20% | 9% | 0% |
| moonshotai/Kimi-K3 | 96 | 0% | 46% | 46% | 85% | 86% | 100% | 46% |
| zai-org/GLM-5.3-Flash | 96 | 0% | 2% | 16% | 56% | 32% | 21% | 1% |

