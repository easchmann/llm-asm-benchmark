## Pass rate (95% CI, bootstrap over kernels)

| model | cells | first try | after repair |
|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 480 | 9% [5-13] | 34% [24-44] |
| claude-haiku-4-5-20251001@thinking-off | 480 | 25% [16-35] | 66% [59-73] |
| claude-haiku-4-5-20251001@thinking-on | 480 | 54% [45-62] | 91% [88-95] |
| claude-haiku-5-5@thinking-on | 480 | 96% [94-98] | 100% [99-100] |
| claude-opus-5-5@thinking-on | 480 | 98% [96-100] | 100% [100-100] |
| claude-opus-5@thinking-off | 480 | 83% [76-90] | 99% [98-100] |
| claude-opus-5@thinking-on | 480 | 99% [97-100] | 100% [100-100] |
| claude-sonnet-5-5@thinking-on | 480 | 97% [94-99] | 99% [98-100] |
| claude-sonnet-5@thinking-off | 480 | 62% [51-73] | 97% [95-99] |
| claude-sonnet-5@thinking-on | 480 | 91% [87-94] | 100% [100-100] |
| DeepSeek-V4-Flash | 480 | 20% [14-26] | 52% [43-60] |
| Kimi-K3 | 480 | 86% [84-89] | 99% [98-100] |
| GLM-5.3-Flash | 480 | 32% [26-39] | 83% [79-88] |

## Level effect: L3 minus L0, paired by kernel (95% CI, bootstrap over kernels)

| model |  | L3 - L0 | kernels better | same | worse |
|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | first try | -7 pts [-12, -2] | 2 | 13 | 9 |
| Qwen3.8-27B@reasoning-none | after repair | -12 pts [-23, +0] | 5 | 5 | 14 |
| claude-haiku-4-5-20251001@thinking-off | first try | +2 pts [-6, +10] | 9 | 11 | 4 |
| claude-haiku-4-5-20251001@thinking-off | after repair | -5 pts [-18, +8] | 6 | 9 | 9 |
| claude-haiku-4-5-20251001@thinking-on | first try | -9 pts [-20, +1] | 7 | 6 | 11 |
| claude-haiku-4-5-20251001@thinking-on | after repair | -7 pts [-15, +1] | 5 | 10 | 9 |
| claude-haiku-5-5@thinking-on | first try | +0 pts [-3, +3] | 2 | 20 | 2 |
| claude-haiku-5-5@thinking-on | after repair | +0 pts [+0, +0] | 0 | 24 | 0 |
| claude-opus-5-5@thinking-on | first try | -2 pts [-4, +0] | 0 | 22 | 2 |
| claude-opus-5-5@thinking-on | after repair | +0 pts [+0, +0] | 0 | 24 | 0 |
| claude-opus-5@thinking-off | first try | -10 pts [-19, -2] | 3 | 13 | 8 |
| claude-opus-5@thinking-off | after repair | -1 pts [-2, +0] | 0 | 23 | 1 |
| claude-opus-5@thinking-on | first try | -1 pts [-2, +0] | 0 | 23 | 1 |
| claude-opus-5@thinking-on | after repair | +0 pts [+0, +0] | 0 | 24 | 0 |
| claude-sonnet-5-5@thinking-on | first try | -2 pts [-5, +2] | 1 | 20 | 3 |
| claude-sonnet-5-5@thinking-on | after repair | -1 pts [-2, +0] | 0 | 23 | 1 |
| claude-sonnet-5@thinking-off | first try | -2 pts [-14, +10] | 8 | 8 | 8 |
| claude-sonnet-5@thinking-off | after repair | +1 pts [-3, +5] | 4 | 17 | 3 |
| claude-sonnet-5@thinking-on | first try | +1 pts [-6, +8] | 7 | 10 | 7 |
| claude-sonnet-5@thinking-on | after repair | +0 pts [+0, +0] | 0 | 24 | 0 |
| DeepSeek-V4-Flash | first try | -2 pts [-13, +9] | 7 | 10 | 7 |
| DeepSeek-V4-Flash | after repair | -8 pts [-18, +2] | 6 | 6 | 12 |
| Kimi-K3 | first try | +3 pts [-6, +13] | 10 | 5 | 9 |
| Kimi-K3 | after repair | -2 pts [-4, +0] | 0 | 22 | 2 |
| GLM-5.3-Flash | first try | +10 pts [+2, +17] | 12 | 10 | 2 |
| GLM-5.3-Flash | after repair | +3 pts [-4, +12] | 10 | 7 | 7 |

## Pass rate by level

| model | L0 first / repaired | L1 first / repaired | L2 first / repaired | L3 first / repaired |
|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 11% / 42% | 10% / 32% | 10% / 30% | 4% / 31% |
| claude-haiku-4-5-20251001@thinking-off | 24% / 70% | 20% / 67% | 29% / 62% | 27% / 65% |
| claude-haiku-4-5-20251001@thinking-on | 56% / 93% | 58% / 92% | 53% / 93% | 47% / 87% |
| claude-haiku-5-5@thinking-on | 97% / 100% | 98% / 100% | 94% / 99% | 97% / 100% |
| claude-opus-5-5@thinking-on | 99% / 100% | 98% / 100% | 99% / 100% | 98% / 100% |
| claude-opus-5@thinking-off | 91% / 100% | 82% / 99% | 79% / 99% | 81% / 99% |
| claude-opus-5@thinking-on | 100% / 100% | 98% / 100% | 98% / 100% | 99% / 100% |
| claude-sonnet-5-5@thinking-on | 98% / 100% | 97% / 100% | 95% / 98% | 97% / 99% |
| claude-sonnet-5@thinking-off | 62% / 97% | 63% / 98% | 64% / 96% | 60% / 98% |
| claude-sonnet-5@thinking-on | 89% / 100% | 92% / 100% | 92% / 100% | 90% / 100% |
| DeepSeek-V4-Flash | 24% / 60% | 18% / 52% | 16% / 43% | 22% / 52% |
| Kimi-K3 | 82% / 100% | 87% / 100% | 90% / 98% | 86% / 98% |
| GLM-5.3-Flash | 29% / 82% | 29% / 85% | 30% / 80% | 39% / 86% |

## Why first tries fail (share of failing first tries)

| model | failing | assemble_fail | wrong: ABI | wrong: small sizes pass | crash SIGSEGV | wrong: all sizes | crash SIGABRT | wrong: out-of-bounds write | link_fail | crash SIGILL | wrong: other | incomplete | wrong: input modified | timeout | crash SIGBUS | extract_fail | crash SIGFPE |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 438 | 68% | 4% | 3% | 11% | 7% | 1% | 1% | 1% | 1% | 1% | 1% | 0% | 0% | 1% | 0% | 0% |
| claude-haiku-4-5-20251001@thinking-off | 360 | 42% | 14% | 7% | 14% | 18% | 1% | 1% | 1% | 0% | 1% | 0% | 1% | 0% | 0% | 0% | 0% |
| claude-haiku-4-5-20251001@thinking-on | 223 | 55% | 12% | 6% | 8% | 17% | 0% | 0% | 0% | 0% | 1% | 0% | 1% | 0% | 0% | 0% | 0% |
| claude-haiku-5-5@thinking-on | 18 | 28% | 44% | 17% | 0% | 11% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% |
| claude-opus-5-5@thinking-on | 8 | 0% | 38% | 38% | 0% | 12% | 0% | 0% | 0% | 0% | 0% | 12% | 0% | 0% | 0% | 0% | 0% |
| claude-opus-5@thinking-off | 81 | 12% | 57% | 15% | 6% | 7% | 0% | 0% | 0% | 0% | 0% | 0% | 2% | 0% | 0% | 0% | 0% |
| claude-opus-5@thinking-on | 7 | 29% | 0% | 14% | 14% | 14% | 0% | 14% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 14% | 0% |
| claude-sonnet-5-5@thinking-on | 16 | 6% | 0% | 19% | 6% | 19% | 0% | 0% | 0% | 0% | 0% | 44% | 0% | 6% | 0% | 0% | 0% |
| claude-sonnet-5@thinking-off | 181 | 33% | 45% | 8% | 9% | 2% | 1% | 0% | 0% | 0% | 1% | 0% | 2% | 1% | 0% | 0% | 0% |
| claude-sonnet-5@thinking-on | 45 | 33% | 53% | 2% | 2% | 9% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% |
| DeepSeek-V4-Flash | 384 | 48% | 8% | 9% | 16% | 5% | 2% | 2% | 4% | 5% | 1% | 0% | 0% | 0% | 0% | 0% | 0% |
| Kimi-K3 | 66 | 30% | 2% | 45% | 5% | 8% | 2% | 3% | 2% | 0% | 3% | 0% | 0% | 2% | 0% | 0% | 0% |
| GLM-5.3-Flash | 327 | 30% | 4% | 28% | 12% | 13% | 6% | 3% | 1% | 1% | 2% | 0% | 0% | 0% | 0% | 0% | 0% |

## Kernels (first try / after repair)

| kernel | source | Qwen3.8-27B@reasoning-none | claude-haiku-4-5-20251001@thinking-off | claude-haiku-4-5-20251001@thinking-on | claude-haiku-5-5@thinking-on | claude-opus-5-5@thinking-on | claude-opus-5@thinking-off | claude-opus-5@thinking-on | claude-sonnet-5-5@thinking-on | claude-sonnet-5@thinking-off | claude-sonnet-5@thinking-on | DeepSeek-V4-Flash | Kimi-K3 | GLM-5.3-Flash |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lower_bound | other | 5% / 10% | 30% / 60% | 15% / 100% | 75% / 100% | 100% / 100% | 70% / 100% | 100% / 100% | 75% / 100% | 15% / 100% | 70% / 100% | 35% / 50% | 85% / 100% | 30% / 85% |
| sort8 | other | 5% / 15% | 0% / 35% | 45% / 85% | 90% / 100% | 100% / 100% | 70% / 100% | 100% / 100% | 100% / 100% | 10% / 95% | 75% / 100% | 10% / 30% | 90% / 100% | 35% / 85% |
| 2mm | PolyBench | 0% / 10% | 5% / 35% | 45% / 80% | 100% / 100% | 100% / 100% | 35% / 100% | 95% / 100% | 100% / 100% | 60% / 95% | 95% / 100% | 0% / 30% | 90% / 100% | 10% / 80% |
| gesummv | PolyBench | 0% / 5% | 20% / 65% | 40% / 85% | 90% / 100% | 95% / 100% | 100% / 100% | 100% / 100% | 95% / 100% | 5% / 100% | 95% / 100% | 5% / 25% | 85% / 100% | 15% / 60% |
| gemver | PolyBench | 0% / 5% | 5% / 60% | 30% / 90% | 100% / 100% | 100% / 100% | 60% / 95% | 100% / 100% | 100% / 100% | 50% / 95% | 100% / 100% | 0% / 15% | 90% / 100% | 15% / 90% |
| heat3d | PolyBench | 0% / 5% | 0% / 40% | 60% / 90% | 100% / 100% | 100% / 100% | 45% / 100% | 100% / 100% | 95% / 100% | 50% / 90% | 100% / 100% | 0% / 15% | 90% / 100% | 15% / 85% |
| 3mm | PolyBench | 0% / 0% | 0% / 60% | 45% / 95% | 100% / 100% | 100% / 100% | 60% / 100% | 95% / 100% | 100% / 100% | 50% / 85% | 90% / 100% | 0% / 10% | 80% / 100% | 35% / 90% |
| bicg | PolyBench | 0% / 20% | 5% / 75% | 25% / 95% | 95% / 100% | 100% / 100% | 90% / 100% | 95% / 100% | 100% / 100% | 55% / 100% | 95% / 100% | 15% / 60% | 85% / 100% | 20% / 85% |
| trmm | PolyBench | 0% / 25% | 10% / 45% | 60% / 90% | 90% / 100% | 85% / 100% | 90% / 100% | 95% / 100% | 90% / 90% | 50% / 100% | 100% / 100% | 10% / 45% | 85% / 100% | 15% / 80% |
| jacobi2d | PolyBench | 0% / 5% | 15% / 65% | 75% / 100% | 90% / 95% | 100% / 100% | 85% / 100% | 100% / 100% | 95% / 100% | 30% / 90% | 90% / 100% | 0% / 35% | 95% / 100% | 20% / 95% |
| gemm | PolyBench | 0% / 25% | 15% / 50% | 20% / 80% | 100% / 100% | 100% / 100% | 80% / 100% | 100% / 100% | 100% / 100% | 55% / 95% | 90% / 100% | 10% / 65% | 80% / 100% | 50% / 95% |
| syrk | PolyBench | 5% / 15% | 5% / 70% | 40% / 90% | 100% / 100% | 100% / 100% | 65% / 90% | 100% / 100% | 100% / 100% | 85% / 95% | 95% / 100% | 15% / 50% | 75% / 100% | 20% / 70% |
| atax | PolyBench | 0% / 50% | 10% / 70% | 40% / 95% | 100% / 100% | 100% / 100% | 100% / 100% | 90% / 100% | 100% / 100% | 70% / 100% | 90% / 100% | 20% / 60% | 90% / 95% | 35% / 70% |
| spmv | other | 10% / 45% | 40% / 55% | 35% / 95% | 100% / 100% | 100% / 100% | 95% / 100% | 100% / 100% | 100% / 100% | 55% / 100% | 75% / 100% | 35% / 70% | 85% / 95% | 50% / 90% |
| mvt | PolyBench | 10% / 50% | 75% / 80% | 60% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 70% / 100% | 80% / 100% | 20% / 70% | 75% / 100% | 20% / 85% |
| popcount | other | 25% / 55% | 45% / 75% | 50% / 60% | 95% / 100% | 80% / 100% | 90% / 100% | 100% / 100% | 75% / 90% | 100% / 100% | 95% / 100% | 40% / 70% | 95% / 100% | 20% / 85% |
| dot | other | 5% / 50% | 25% / 60% | 60% / 85% | 95% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 85% / 100% | 100% / 100% | 45% / 55% | 85% / 95% | 30% / 80% |
| nrm2 | other | 35% / 70% | 10% / 65% | 60% / 90% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 85% / 95% | 100% / 100% | 35% / 75% | 75% / 90% | 35% / 85% |
| jacobi1d | PolyBench | 15% / 45% | 35% / 80% | 95% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 95% / 100% | 55% / 100% | 100% / 100% | 10% / 50% | 90% / 100% | 45% / 90% |
| trmv | other | 10% / 55% | 70% / 95% | 75% / 95% | 95% / 100% | 100% / 100% | 90% / 100% | 100% / 100% | 100% / 100% | 65% / 95% | 70% / 100% | 50% / 80% | 90% / 100% | 30% / 75% |
| scal | other | 20% / 75% | 40% / 80% | 70% / 95% | 95% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 95% / 100% | 100% / 100% | 35% / 65% | 80% / 100% | 55% / 90% |
| axpy | other | 20% / 45% | 45% / 80% | 70% / 100% | 100% / 100% | 100% / 100% | 95% / 100% | 95% / 100% | 100% / 100% | 100% / 100% | 90% / 100% | 35% / 75% | 95% / 100% | 55% / 100% |
| histogram | other | 20% / 60% | 10% / 90% | 90% / 100% | 100% / 100% | 100% / 100% | 95% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 90% / 100% | 25% / 75% | 100% / 100% | 75% / 100% |
| prefix_sum | other | 25% / 75% | 85% / 95% | 80% / 100% | 100% / 100% | 100% / 100% | 80% / 100% | 100% / 100% | 100% / 100% | 100% / 100% | 90% / 100% | 30% / 70% | 80% / 100% | 35% / 50% |

## PolyBench-derived vs other kernels (first try / after repair)

| model | PolyBench | other |
|---|---|---|
| Qwen3.8-27B@reasoning-none | 2% / 20% (n=260) | 16% / 50% (n=220) |
| claude-haiku-4-5-20251001@thinking-off | 15% / 61% (n=260) | 36% / 72% (n=220) |
| claude-haiku-4-5-20251001@thinking-on | 49% / 92% (n=260) | 59% / 91% (n=220) |
| claude-haiku-5-5@thinking-on | 97% / 100% (n=260) | 95% / 100% (n=220) |
| claude-opus-5-5@thinking-on | 98% / 100% (n=260) | 98% / 100% (n=220) |
| claude-opus-5@thinking-off | 78% / 99% (n=260) | 90% / 100% (n=220) |
| claude-opus-5@thinking-on | 98% / 100% (n=260) | 100% / 100% (n=220) |
| claude-sonnet-5-5@thinking-on | 98% / 99% (n=260) | 95% / 99% (n=220) |
| claude-sonnet-5@thinking-off | 53% / 96% (n=260) | 74% / 99% (n=220) |
| claude-sonnet-5@thinking-on | 94% / 100% (n=260) | 87% / 100% (n=220) |
| DeepSeek-V4-Flash | 8% / 41% (n=260) | 34% / 65% (n=220) |
| Kimi-K3 | 85% / 100% (n=260) | 87% / 98% (n=220) |
| GLM-5.3-Flash | 24% / 83% (n=260) | 41% / 84% (n=220) |

## Most common assembler errors (first try)

| count | model | error |
|---|---|---|
| 60 | claude-haiku-4-5-20251001@thinking-on | operand size mismatch for `mov' |
| 40 | claude-haiku-4-5-20251001@thinking-off | operand size mismatch for `mov' |
| 31 | Qwen3.8-27B@reasoning-none | operand size mismatch for `mov' |
| 30 | DeepSeek-V4-Flash | operand type mismatch for `vmovsd' |
| 28 | claude-haiku-4-5-20251001@thinking-off | junk `X` after expression |
| 25 | claude-haiku-4-5-20251001@thinking-off | operand size mismatch for `test' |
| 23 | GLM-5.3-Flash | operand size mismatch for `mov' |
| 23 | Qwen3.8-27B@reasoning-none | incorrect register `X` used with `X` suffix |
| 23 | claude-haiku-4-5-20251001@thinking-on | operand size mismatch for `xor' |
| 18 | DeepSeek-V4-Flash | bad register name `X` |
| 18 | Qwen3.8-27B@reasoning-none | operand type mismatch for `movsd' |
| 18 | Qwen3.8-27B@reasoning-none | bad register name `X` |
| 18 | claude-haiku-4-5-20251001@thinking-off | operand size mismatch for `xor' |
| 17 | claude-haiku-4-5-20251001@thinking-on | operand size mismatch for `test' |
| 16 | Qwen3.8-27B@reasoning-none | junk `X` after expression |

## Checks on passing code (final round)

| model | passing cells | with call instruction | immediate equal to a test size (check by hand) |
|---|---|---|---|
| Qwen3.8-27B@reasoning-none | 163 | 0 | 3 |
| claude-haiku-4-5-20251001@thinking-off | 317 | 1 | 9 |
| claude-haiku-4-5-20251001@thinking-on | 439 | 5 | 6 |
| claude-haiku-5-5@thinking-on | 479 | 0 | 33 |
| claude-opus-5-5@thinking-on | 480 | 0 | 37 |
| claude-opus-5@thinking-off | 477 | 9 | 35 |
| claude-opus-5@thinking-on | 480 | 0 | 48 |
| claude-sonnet-5-5@thinking-on | 476 | 8 | 41 |
| claude-sonnet-5@thinking-off | 466 | 31 | 16 |
| claude-sonnet-5@thinking-on | 480 | 0 | 24 |
| DeepSeek-V4-Flash | 249 | 1 | 4 |
| Kimi-K3 | 475 | 0 | 30 |
| GLM-5.3-Flash | 400 | 15 | 36 |

