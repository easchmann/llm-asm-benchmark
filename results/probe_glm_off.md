## Requests (prompt: What is 17 × 23? Answer with the number only.)

| model | setting | try | status | finish | reasoning tokens | completion tokens | reasoning_content chars | <think> in content | answer (end) | s | cost | underlying model |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| zai-org/GLM-5.3-Flash | default | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.3 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | default | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | chat_template_kwargs | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | chat_template_kwargs | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | chat_template_kwargs, allowed | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | chat_template_kwargs, allowed | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | extra_body | 0 | ok | stop | 9 | 13 | 14 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | extra_body | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | thinking disabled | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | thinking disabled | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | /nothink | 0 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | /nothink | 1 | ok | stop | 8 | 12 | 13 | no | 391 | 0.2 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | effort minimal | 0 | ok | stop | 137 | 141 | 299 | no | 391 | 0.8 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |
| zai-org/GLM-5.3-Flash | effort minimal | 1 | ok | stop | 51 | 55 | 122 | no | 391 | 0.4 | $0.0000 | hosted_vllm/zai-org/GLM-5.3-Flash |

## Per model and setting

| model | setting | answered | reasoning tokens | completion tokens | reasoning_content chars | answers with <think> | slowest s | cost per request |
|---|---|---|---|---|---|---|---|---|
| zai-org/GLM-5.3-Flash | default | 2/2 | 8 | 12 | 13 | 0 | 0.3 | $0.0000 |
| zai-org/GLM-5.3-Flash | chat_template_kwargs | 2/2 | 8 | 12 | 13 | 0 | 0.2 | $0.0000 |
| zai-org/GLM-5.3-Flash | chat_template_kwargs, allowed | 2/2 | 8 | 12 | 13 | 0 | 0.2 | $0.0000 |
| zai-org/GLM-5.3-Flash | extra_body | 2/2 | 8 | 12 | 14 | 0 | 0.2 | $0.0000 |
| zai-org/GLM-5.3-Flash | thinking disabled | 2/2 | 8 | 12 | 13 | 0 | 0.2 | $0.0000 |
| zai-org/GLM-5.3-Flash | /nothink | 2/2 | 8 | 12 | 13 | 0 | 0.2 | $0.0000 |
| zai-org/GLM-5.3-Flash | effort minimal | 2/2 | 94 | 98 | 210 | 0 | 0.8 | $0.0000 |

Total cost: $0.0001 (log: /home/elena.aschmann/llm-asm-controls/results/probe_reasoning.jsonl)
