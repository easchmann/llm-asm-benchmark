## Failure stages that exist only for C (share of cells, first try / any round)

| model | stage | first try | records, all rounds |
|---|---|---|---|
| Qwen3.8-27B@reasoning-none | forbidden | 0% | 0 |
| Qwen3.8-27B@reasoning-none | compile_fail | 0% | 3 |
| DeepSeek-V4-Flash | forbidden | 0% | 0 |
| DeepSeek-V4-Flash | compile_fail | 22% | 110 |
| Kimi-K3 | forbidden | 0% | 0 |
| Kimi-K3 | compile_fail | 1% | 4 |
| GLM-5.3-Flash | forbidden | 0% | 0 |
| GLM-5.3-Flash | compile_fail | 9% | 41 |

## Compiler errors in desc_viac, first error line (all rounds)

| model | records | first error |
|---|---|---|
| Qwen3.8-27B@reasoning-none | 2 | error: implicit declaration of function ‘malloc’ [-Wimplicit-function- |
| Qwen3.8-27B@reasoning-none | 1 | error: subscripted value is neither array nor pointer nor vector |
| DeepSeek-V4-Flash | 107 | error: stray ‘`’ in program |
| DeepSeek-V4-Flash | 2 | error: implicit declaration of function ‘malloc’ [-Wimplicit-function- |
| DeepSeek-V4-Flash | 1 | error: expected ‘;’ before ‘typedef’ |
| Kimi-K3 | 3 | error: ‘size_t’ undeclared (first use in this function) |
| Kimi-K3 | 1 | error: ‘NULL’ undeclared (first use in this function) |
| GLM-5.3-Flash | 36 | error: ‘size_t’ undeclared (first use in this function) |
| GLM-5.3-Flash | 1 | error: ‘#pragma GCC optimize’ is not allowed inside functions |
| GLM-5.3-Flash | 1 | error: assignment of read-only location ‘*(a + (sizetype)((long unsign |

## Answers with an unclosed code block, re-scored offline with the block closed (first try)

| run | model | first tries | recorded as | re-scored |
|---|---|---|---|---|
| desc_full | Qwen3.8-27B@reasoning-none | 5 | incomplete | incomplete 3, assemble_fail 2 |
| desc_viac | DeepSeek-V4-Flash | 106 | compile_fail | pass 92, wrong 8, compile_fail 6 |

## Pass rate, C - asm, as recorded (95% CI, bootstrap over kernels)

| model | level | cells | first try desc_full | first try desc_viac | C - asm | after repair desc_full | after repair desc_viac | C - asm |
|---|---|---|---|---|---|---|---|---|
| all | all | 1920 | 37% | 90% | +54 pts [+51, +56] | 67% | 99% | +32 pts [+28, +37] |
| Qwen3.8-27B@reasoning-none | all | 480 | 9% | 98% | +89 pts [+85, +94] | 34% | 99% | +65 pts [+56, +75] |
| DeepSeek-V4-Flash | all | 480 | 20% | 76% | +56 pts [+49, +62] | 52% | 99% | +47 pts [+39, +55] |
| Kimi-K3 | all | 480 | 86% | 99% | +12 pts [+10, +15] | 99% | 100% | +1 pts [+0, +2] |
| GLM-5.3-Flash | all | 480 | 32% | 89% | +57 pts [+50, +63] | 83% | 100% | +16 pts [+12, +21] |

## Pass rate, C - asm, unclosed code blocks re-scored (95% CI, bootstrap over kernels)

| model | level | cells | first try desc_full | first try desc_viac | C - asm | after repair desc_full | after repair desc_viac | C - asm |
|---|---|---|---|---|---|---|---|---|
| all | all | 1920 | 37% | 95% | +58 pts [+56, +62] | 67% | 99% | +32 pts [+28, +37] |
| Qwen3.8-27B@reasoning-none | all | 480 | 9% | 98% | +89 pts [+85, +94] | 34% | 99% | +65 pts [+56, +75] |
| DeepSeek-V4-Flash | all | 480 | 20% | 95% | +75 pts [+69, +82] | 52% | 99% | +47 pts [+39, +56] |
| Kimi-K3 | all | 480 | 86% | 99% | +12 pts [+10, +15] | 99% | 100% | +1 pts [+0, +2] |
| GLM-5.3-Flash | all | 480 | 32% | 89% | +57 pts [+50, +63] | 83% | 100% | +16 pts [+12, +21] |

## Per kernel, all models (unclosed code blocks re-scored)

| kernel | cells | first try desc_full | first try desc_viac | after repair desc_full | after repair desc_viac |
|---|---|---|---|---|---|
| 2mm | 80 | 25% | 95% | 55% | 100% |
| gemver | 80 | 26% | 82% | 52% | 100% |
| gesummv | 80 | 26% | 95% | 48% | 100% |
| heat3d | 80 | 26% | 88% | 51% | 99% |
| trmm | 80 | 28% | 71% | 62% | 95% |
| 3mm | 80 | 29% | 100% | 50% | 100% |
| jacobi2d | 80 | 29% | 99% | 59% | 100% |
| syrk | 80 | 29% | 92% | 59% | 100% |
| bicg | 80 | 30% | 98% | 66% | 100% |
| mvt | 80 | 31% | 96% | 76% | 100% |
| gemm | 80 | 35% | 88% | 71% | 100% |
| sort8 | 80 | 35% | 94% | 57% | 96% |
| atax | 80 | 36% | 98% | 69% | 100% |
| lower_bound | 80 | 39% | 100% | 61% | 100% |
| jacobi1d | 80 | 40% | 98% | 71% | 98% |
| dot | 80 | 41% | 100% | 70% | 100% |
| prefix_sum | 80 | 42% | 100% | 74% | 100% |
| nrm2 | 80 | 45% | 99% | 80% | 100% |
| popcount | 80 | 45% | 99% | 78% | 100% |
| spmv | 80 | 45% | 100% | 75% | 100% |
| trmv | 80 | 45% | 96% | 78% | 100% |
| scal | 80 | 48% | 100% | 82% | 100% |
| axpy | 80 | 51% | 99% | 80% | 100% |
| histogram | 80 | 55% | 100% | 84% | 100% |

## Cells that fail after repair (unclosed code blocks re-scored)

| model | run | cells | kernels (top 5) | final stage |
|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | desc_full | 317 | 3mm 20, gemver 19, gesummv 19, heat3d 19, jacobi2d 19 | assemble_fail 119, crash 99, wrong 95, link_fail 2, timeout 1, incomplete 1 |
| Qwen3.8-27B@reasoning-none | desc_viac | 3 | jacobi1d 2, sort8 1 | wrong 3 |
| DeepSeek-V4-Flash | desc_full | 231 | 3mm 18, gemver 17, heat3d 17, gesummv 15, 2mm 14 | wrong 115, crash 79, assemble_fail 37 |
| DeepSeek-V4-Flash | desc_viac | 5 | trmm 3, sort8 2 | wrong 5 |
| Kimi-K3 | desc_full | 5 | nrm2 2, atax 1, dot 1, spmv 1 | no_answer 4, wrong 1 |
| Kimi-K3 | desc_viac | 0 | - | - |
| GLM-5.3-Flash | desc_full | 80 | prefix_sum 10, gesummv 8, atax 6, syrk 6, trmv 5 | wrong 62, crash 14, no_answer 2, assemble_fail 2 |
| GLM-5.3-Flash | desc_viac | 2 | heat3d 1, trmm 1 | wrong 2 |

Cells solved in desc_full but not in desc_viac: 4 (Qwen3.8-27B@reasoning-none jacobi1d L3 rep0, Qwen3.8-27B@reasoning-none jacobi1d L3 rep3, DeepSeek-V4-Flash trmm L1 rep2, GLM-5.3-Flash trmm L2 rep0)

## Per level, all models (unclosed code blocks re-scored)

| model | level | cells | first try desc_full | first try desc_viac | C - asm | after repair desc_full | after repair desc_viac | C - asm |
|---|---|---|---|---|---|---|---|---|
| all | L0 | 480 | 37% | 97% | +60 pts [+56, +65] | 71% | 100% | +29 pts [+24, +34] |
| all | L1 | 480 | 36% | 95% | +59 pts [+55, +63] | 67% | 100% | +32 pts [+28, +37] |
| all | L2 | 480 | 36% | 94% | +57 pts [+53, +62] | 63% | 99% | +36 pts [+31, +41] |
| all | L3 | 480 | 38% | 95% | +57 pts [+53, +61] | 67% | 99% | +33 pts [+28, +38] |

