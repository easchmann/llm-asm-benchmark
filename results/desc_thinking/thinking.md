## Variants

| run | model | base model | underlying model | cells | no answer | reasoning tokens per call | reasoning |
|---|---|---|---|---|---|---|---|
| desc_full | Qwen/Qwen3.8-27B@reasoning-none | Qwen/Qwen3.8-27B | hosted_vllm/Qwen/Qwen3.8-27B | 480 | 0 | 0 | off |
| desc_full | deepseek-ai/DeepSeek-V4-Flash | deepseek-ai/DeepSeek-V4-Flash | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash | 480 | 0 | 0 | off |
| desc_full | moonshotai/Kimi-K3 | moonshotai/Kimi-K3 | litellm_proxy/kimi-k3 | 480 | 0 | 6631 | on |
| desc_full | zai-org/GLM-5.3-Flash | zai-org/GLM-5.3-Flash | hosted_vllm/zai-org/GLM-5.3-Flash | 480 | 0 | 3795 | on |
| desc_thinking | Qwen/Qwen3.8-27B@reasoning-low | Qwen/Qwen3.8-27B | hosted_vllm/Qwen/Qwen3.8-27B | 480 | 9 | 8617 | on |
| desc_thinking | Qwen/Qwen3.8-27B@reasoning-none | Qwen/Qwen3.8-27B | hosted_vllm/Qwen/Qwen3.8-27B | 480 | 0 | 0 | off |
| desc_thinking | deepseek-ai/DeepSeek-V4-Flash | deepseek-ai/DeepSeek-V4-Flash | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash | 480 | 0 | 0 | off |
| desc_thinking | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium | deepseek-ai/DeepSeek-V4-Flash | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash | 480 | 0 | 9300 | on |
| desc_thinking | zai-org/GLM-5.3-Flash | zai-org/GLM-5.3-Flash | hosted_vllm/zai-org/GLM-5.3-Flash | 96 | 0 | 7163 | on |
| desc_thinking | zai-org/GLM-5.3-Flash@reasoning-none | zai-org/GLM-5.3-Flash | hosted_vllm/zai-org/GLM-5.3-Flash | 427 | 23 | 35871 | on |

## Same variant in two runs, later - earlier (drift check, 95% CI, bootstrap over kernels)

| model | earlier | later | cells | first try earlier | first try later | later - earlier | after repair earlier | after repair later | later - earlier |
|---|---|---|---|---|---|---|---|---|---|
| deepseek-ai/DeepSeek-V4-Flash | desc_full | desc_thinking | 480 | 20% | 19% | -1 pts [-5, +3] | 52% | 55% | +3 pts [-2, +8] |
| zai-org/GLM-5.3-Flash | desc_full | desc_thinking | 96 | 33% | 32% | -1 pts [-9, +7] | 85% | 90% | +4 pts [-6, +14] |
| Qwen/Qwen3.8-27B@reasoning-none | desc_full | desc_thinking | 480 | 9% | 11% | +2 pts [-1, +6] | 34% | 31% | -3 pts [-9, +3] |

## Pass rate, on - off (95% CI, bootstrap over kernels)

| off | on | level | cells | first try off | first try on | on - off | after repair off | after repair on | on - off |
|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | all | 471 | 9% | 8% | -0 pts [-5, +4] | 34% | 52% | +17 pts [+8, +28] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L0 | 119 | 11% | 8% | -3 pts [-10, +4] | 43% | 55% | +12 pts [-1, +24] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L1 | 117 | 10% | 8% | -3 pts [-11, +6] | 32% | 44% | +12 pts [-4, +30] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L2 | 118 | 10% | 8% | -2 pts [-9, +6] | 31% | 55% | +25 pts [+13, +36] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L3 | 117 | 4% | 10% | +6 pts [-1, +14] | 32% | 53% | +21 pts [+9, +34] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | all | 471 | 11% | 8% | -3 pts [-8, +4] | 31% | 52% | +20 pts [+12, +29] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L0 | 119 | 12% | 8% | -4 pts [-11, +3] | 31% | 55% | +24 pts [+13, +34] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L1 | 117 | 12% | 8% | -4 pts [-13, +4] | 36% | 44% | +9 pts [-8, +26] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L2 | 118 | 10% | 8% | -2 pts [-10, +6] | 29% | 55% | +26 pts [+15, +36] |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | L3 | 117 | 10% | 10% | +0 pts [-8, +8] | 30% | 53% | +23 pts [+12, +35] |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | all | 480 | 20% | 57% | +37 pts [+33, +42] | 52% | 97% | +45 pts [+38, +52] |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L0 | 120 | 24% | 60% | +36 pts [+24, +48] | 60% | 98% | +38 pts [+29, +47] |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L1 | 120 | 18% | 53% | +36 pts [+27, +44] | 52% | 97% | +45 pts [+36, +55] |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L2 | 120 | 16% | 55% | +39 pts [+27, +52] | 43% | 97% | +53 pts [+42, +65] |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L3 | 120 | 22% | 61% | +38 pts [+29, +48] | 52% | 96% | +43 pts [+33, +53] |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | all | 480 | 19% | 57% | +38 pts [+32, +45] | 55% | 97% | +42 pts [+34, +50] |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L0 | 120 | 22% | 60% | +38 pts [+27, +49] | 58% | 98% | +39 pts [+31, +48] |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L1 | 120 | 17% | 53% | +37 pts [+23, +48] | 54% | 97% | +43 pts [+32, +53] |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L2 | 120 | 17% | 55% | +38 pts [+28, +50] | 52% | 97% | +44 pts [+36, +52] |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | L3 | 120 | 21% | 61% | +40 pts [+29, +50] | 55% | 96% | +41 pts [+27, +55] |

## First try: pass and failure types (share of cells)

| variant | reasoning | cells | pass | assembler error | crash or timeout | ABI or memory violation | wrong values, larger sizes only | wrong values, all sizes | other (link, unfinished) |
|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | off | 471 | 9% | 62% | 12% | 5% | 3% | 7% | 2% |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 8% | 75% | 7% | 2% | 3% | 3% | 1% |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | off | 471 | 11% | 63% | 12% | 4% | 2% | 6% | 2% |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 8% | 75% | 7% | 2% | 3% | 3% | 1% |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | off | 480 | 20% | 38% | 19% | 8% | 7% | 4% | 3% |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 57% | 18% | 9% | 4% | 7% | 3% | 1% |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | off | 480 | 19% | 37% | 22% | 8% | 8% | 4% | 2% |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 57% | 18% | 9% | 4% | 7% | 3% | 1% |

## After repair: pass and failure types (share of cells)

| variant | reasoning | cells | pass | assembler error | crash or timeout | ABI or memory violation | wrong values, larger sizes only | wrong values, all sizes | other (link, unfinished) |
|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | off | 471 | 34% | 25% | 20% | 2% | 5% | 12% | 1% |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 52% | 7% | 17% | 2% | 9% | 7% | 6% |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | off | 471 | 31% | 28% | 22% | 5% | 4% | 9% | 1% |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 52% | 7% | 17% | 2% | 9% | 7% | 6% |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | off | 480 | 52% | 8% | 16% | 7% | 9% | 8% | 0% |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 97% | 0% | 2% | 0% | 0% | 0% | 0% |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | off | 480 | 55% | 7% | 16% | 7% | 10% | 5% | 0% |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 97% | 0% | 2% | 0% | 0% | 0% | 0% |

## Tokens, time and cost per cell (all rounds)

| variant | reasoning | cells | calls per cell | reasoning tokens | completion tokens | s per cell | slowest call s | calls >= 600 s | cost per cell | cost per solved cell |
|---|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B@reasoning-none (desc_full) | off | 471 | 3.42 | 0 | 7845 | 71 | 194 | 0 | $0.0016 | $0.0048 |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 3.65 | 32159 | 37237 | 286 | 389 | 0 | $0.0054 | $0.0105 |
| Qwen/Qwen3.8-27B@reasoning-none (desc_thinking) | off | 471 | 3.41 | 0 | 8038 | 64 | 181 | 0 | $0.0017 | $0.0053 |
| Qwen/Qwen3.8-27B@reasoning-low (desc_thinking) | on | 471 | 3.65 | 32159 | 37237 | 286 | 389 | 0 | $0.0054 | $0.0105 |
| deepseek-ai/DeepSeek-V4-Flash (desc_full) | off | 480 | 2.79 | 0 | 4955 | 13 | 100 | 0 | $0.0076 | $0.0147 |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 1.52 | 14144 | 16082 | 42 | 110 | 0 | $0.0243 | $0.0252 |
| deepseek-ai/DeepSeek-V4-Flash (desc_thinking) | off | 480 | 2.71 | 0 | 4606 | 10 | 43 | 0 | $0.0071 | $0.0129 |
| deepseek-ai/DeepSeek-V4-Flash@reasoning-medium (desc_thinking) | on | 480 | 1.52 | 14144 | 16082 | 42 | 110 | 0 | $0.0243 | $0.0252 |

