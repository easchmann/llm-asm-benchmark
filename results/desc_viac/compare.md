A = desc_full, B = desc_viac: 1920 cells answered in both, 0 only in A, 0 only in B

## Pass rate, B - A (95% CI, bootstrap over kernels)

| model | level | cells | first try A | first try B | B - A | after repair A | after repair B | B - A |
|---|---|---|---|---|---|---|---|---|
| all | all | 1920 | 37% | 90% | +54 pts [+51, +56] | 67% | 99% | +32 pts [+28, +37] |
| Qwen3.8-27B@reasoning-none | all | 480 | 9% | 98% | +89 pts [+85, +94] | 34% | 99% | +65 pts [+56, +75] |
| Qwen3.8-27B@reasoning-none | L0 | 120 | 11% | 99% | +88 pts [+83, +93] | 42% | 100% | +57 pts [+46, +70] |
| Qwen3.8-27B@reasoning-none | L1 | 120 | 10% | 98% | +88 pts [+80, +96] | 32% | 100% | +68 pts [+56, +79] |
| Qwen3.8-27B@reasoning-none | L2 | 120 | 10% | 98% | +88 pts [+82, +94] | 30% | 99% | +69 pts [+58, +80] |
| Qwen3.8-27B@reasoning-none | L3 | 120 | 4% | 97% | +92 pts [+86, +98] | 31% | 98% | +68 pts [+54, +79] |
| DeepSeek-V4-Flash | all | 480 | 20% | 76% | +56 pts [+49, +62] | 52% | 99% | +47 pts [+39, +55] |
| DeepSeek-V4-Flash | L0 | 120 | 24% | 82% | +57 pts [+45, +68] | 60% | 100% | +40 pts [+30, +51] |
| DeepSeek-V4-Flash | L1 | 120 | 18% | 79% | +62 pts [+52, +71] | 52% | 98% | +47 pts [+38, +57] |
| DeepSeek-V4-Flash | L2 | 120 | 16% | 72% | +57 pts [+43, +69] | 43% | 98% | +55 pts [+44, +66] |
| DeepSeek-V4-Flash | L3 | 120 | 22% | 71% | +48 pts [+34, +62] | 52% | 98% | +46 pts [+35, +57] |
| Kimi-K3 | all | 480 | 86% | 99% | +12 pts [+10, +15] | 99% | 100% | +1 pts [+0, +2] |
| Kimi-K3 | L0 | 120 | 82% | 98% | +16 pts [+10, +22] | 100% | 100% | +0 pts [+0, +0] |
| Kimi-K3 | L1 | 120 | 87% | 99% | +12 pts [+7, +19] | 100% | 100% | +0 pts [+0, +0] |
| Kimi-K3 | L2 | 120 | 90% | 99% | +9 pts [+5, +14] | 98% | 100% | +3 pts [+0, +6] |
| Kimi-K3 | L3 | 120 | 86% | 98% | +12 pts [+6, +18] | 98% | 100% | +2 pts [+0, +4] |
| GLM-5.3-Flash | all | 480 | 32% | 89% | +57 pts [+50, +63] | 83% | 100% | +16 pts [+12, +21] |
| GLM-5.3-Flash | L0 | 120 | 29% | 91% | +62 pts [+53, +71] | 82% | 100% | +18 pts [+10, +25] |
| GLM-5.3-Flash | L1 | 120 | 29% | 88% | +59 pts [+46, +71] | 85% | 100% | +15 pts [+7, +23] |
| GLM-5.3-Flash | L2 | 120 | 30% | 86% | +56 pts [+44, +67] | 80% | 98% | +18 pts [+10, +27] |
| GLM-5.3-Flash | L3 | 120 | 39% | 91% | +52 pts [+42, +61] | 86% | 100% | +14 pts [+9, +21] |
| all | L0 | 480 | 37% | 92% | +56 pts [+51, +60] | 71% | 100% | +29 pts [+24, +34] |
| all | L1 | 480 | 36% | 91% | +55 pts [+51, +59] | 67% | 100% | +32 pts [+28, +37] |
| all | L2 | 480 | 36% | 89% | +52 pts [+48, +57] | 63% | 99% | +36 pts [+31, +41] |
| all | L3 | 480 | 38% | 89% | +51 pts [+47, +55] | 67% | 99% | +32 pts [+27, +38] |

## First try: pass and failure types (share of cells)

| model | run | cells | pass | assembler error | crash or timeout | ABI or memory violation | wrong values, larger sizes only | wrong values, all sizes | other (link, unfinished) |
|---|---|---|---|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | desc_full | 480 | 9% | 62% | 12% | 5% | 3% | 7% | 2% |
| Qwen3.8-27B@reasoning-none | desc_viac | 480 | 98% | 0% | 0% | 0% | 1% | 1% | 0% |
| DeepSeek-V4-Flash | desc_full | 480 | 20% | 38% | 19% | 8% | 7% | 4% | 3% |
| DeepSeek-V4-Flash | desc_viac | 480 | 76% | 22% | 0% | 0% | 1% | 1% | 0% |
| Kimi-K3 | desc_full | 480 | 86% | 4% | 1% | 1% | 6% | 1% | 0% |
| Kimi-K3 | desc_viac | 480 | 99% | 1% | 0% | 0% | 1% | 0% | 0% |
| GLM-5.3-Flash | desc_full | 480 | 32% | 20% | 13% | 5% | 19% | 10% | 1% |
| GLM-5.3-Flash | desc_viac | 480 | 89% | 9% | 0% | 0% | 1% | 1% | 0% |
| all | desc_full | 1920 | 37% | 31% | 11% | 5% | 9% | 6% | 2% |
| all | desc_viac | 1920 | 90% | 8% | 0% | 0% | 1% | 1% | 0% |

## Cost and time per cell (all rounds, the model's own plan call included)

| model | run | cells | calls per cell | cost per cell | of which plan | s per cell | of which plan | cost per solved cell |
|---|---|---|---|---|---|---|---|---|
| Qwen3.8-27B@reasoning-none | desc_full | 480 | 3.42 | $0.0017 | - | 71 | - | $0.0049 |
| Qwen3.8-27B@reasoning-none | desc_viac | 480 | 1.02 | $0.0001 | - | 2 | - | $0.0001 |
| DeepSeek-V4-Flash | desc_full | 480 | 2.79 | $0.0076 | - | 13 | - | $0.0147 |
| DeepSeek-V4-Flash | desc_viac | 480 | 1.26 | $0.0005 | - | 1 | - | $0.0005 |
| Kimi-K3 | desc_full | 480 | 1.14 | $0.0936 | - | 323 | - | $0.0946 |
| Kimi-K3 | desc_viac | 480 | 1.01 | $0.0142 | - | 50 | - | $0.0142 |
| GLM-5.3-Flash | desc_full | 480 | 2.08 | $0.0061 | - | 169 | - | $0.0074 |
| GLM-5.3-Flash | desc_viac | 480 | 1.12 | $0.0002 | - | 10 | - | $0.0002 |

