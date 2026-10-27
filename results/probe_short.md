## Requests (prompt: What is 17 × 23? Answer with the number only.)

| model | reasoning_effort | try | status | finish | reasoning tokens | completion tokens | reasoning_content chars | <think> in content | answer (end) | s | cost | underlying model |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| zai-org/GLM-5.3-Flash | default | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | default | 1 | ok | stop | 17 | 21 | 88 | no | 391 | 0.3 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | none | 0 | ok | stop | 43 | 47 | 83 | no | 391 | 0.4 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | none | 1 | ok | stop | 37 | 41 | 61 | no | 391 | 0.3 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | low | 0 | ok | stop | 6 | 10 | 9 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | low | 1 | ok | stop | 6 | 10 | 9 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| deepseek-ai/DeepSeek-V4-Flash | default | 0 | ok | stop | 0 | 2 | 0 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | default | 1 | ok | stop | 0 | 2 | 0 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | low | 0 | ok | stop | 13 | 16 | 38 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | low | 1 | ok | stop | 17 | 20 | 53 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | medium | 0 | ok | stop | 24 | 27 | 87 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | medium | 1 | ok | stop | 17 | 20 | 57 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | high | 0 | ok | stop | 17 | 20 | 57 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| deepseek-ai/DeepSeek-V4-Flash | high | 1 | ok | stop | 13 | 16 | 38 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| RiVault/Reasoning-Medium | default | 0 | ok | stop | 19 | 22 | 64 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| RiVault/Reasoning-Medium | default | 1 | ok | stop | 14 | 17 | 43 | no | 391 | 0.2 | $0.0000 | hosted_vllm/deepseek-ai/DeepSeek-V4-Flash |
| Qwen/Qwen3.8-27B | none | 0 | ok | stop | 0 | 4 | 0 | no | 391 | 0.3 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | none | 1 | ok | stop | 0 | 4 | 0 | no | 391 | 0.3 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | low | 0 | ok | stop | 57 | 63 | 117 | no | 391 | 0.6 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | low | 1 | ok | stop | 57 | 63 | 117 | no | 391 | 0.6 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | default | 0 | ok | stop | 61 | 67 | 163 | no | 391 | 0.7 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | default | 1 | ok | stop | 60 | 66 | 158 | no | 391 | 0.6 | $0.0000 | hosted_vllm/Qwen/Qwen3.8-27B |
| moonshotai/Kimi-K3 | none | 0 | ok | stop | 28 | 39 | 43 | no | 391 | 1.8 | $0.0004 | litellm_proxy/kimi-k3 |

## Per model and setting

| model | reasoning_effort | answered | reasoning tokens | completion tokens | reasoning_content chars | slowest s | cost per request |
|---|---|---|---|---|---|---|---|
| zai-org/GLM-5.3-Flash | default | 2/2 | 12 | 16 | 50 | 0.3 | $0.0000 |
| zai-org/GLM-5.3-Flash | none | 2/2 | 40 | 44 | 72 | 0.4 | $0.0000 |
| zai-org/GLM-5.3-Flash | low | 2/2 | 6 | 10 | 9 | 0.2 | $0.0000 |
| deepseek-ai/DeepSeek-V4-Flash | default | 2/2 | 0 | 2 | 0 | 0.2 | $0.0000 |
| deepseek-ai/DeepSeek-V4-Flash | low | 2/2 | 15 | 18 | 46 | 0.2 | $0.0000 |
| deepseek-ai/DeepSeek-V4-Flash | medium | 2/2 | 20 | 24 | 72 | 0.2 | $0.0000 |
| deepseek-ai/DeepSeek-V4-Flash | high | 2/2 | 15 | 18 | 48 | 0.2 | $0.0000 |
| RiVault/Reasoning-Medium | default | 2/2 | 16 | 20 | 54 | 0.2 | $0.0000 |
| Qwen/Qwen3.8-27B | none | 2/2 | 0 | 4 | 0 | 0.3 | $0.0000 |
| Qwen/Qwen3.8-27B | low | 2/2 | 57 | 63 | 117 | 0.6 | $0.0000 |
| Qwen/Qwen3.8-27B | default | 2/2 | 60 | 66 | 160 | 0.7 | $0.0000 |
| moonshotai/Kimi-K3 | none | 1/1 | 28 | 39 | 43 | 1.8 | $0.0004 |

Total cost: $0.0008 (log: /home/elena.aschmann/llm-asm-controls/results/probe_reasoning.jsonl)
