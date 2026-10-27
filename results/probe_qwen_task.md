## Requests (prompt: gemm L3)

| model | reasoning_effort | try | status | finish | reasoning tokens | completion tokens | reasoning_content chars | <think> in content | answer (end) | s | cost | underlying model |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B | low | 0 | ok | stop | 21910 | 23667 | 56568 | no | rbx ret .size gemm, .-gemm ``` | 159.9 | $0.0032 | hosted_vllm/Qwen/Qwen3.8-27B |
| Qwen/Qwen3.8-27B | low | 1 | ok | stop | 12502 | 13996 | 31300 | no | rbp ret .size gemm, .-gemm ``` | 93.9 | $0.0019 | hosted_vllm/Qwen/Qwen3.8-27B |

## Per model and setting

| model | reasoning_effort | answered | reasoning tokens | completion tokens | reasoning_content chars | slowest s | cost per request |
|---|---|---|---|---|---|---|---|
| Qwen/Qwen3.8-27B | low | 2/2 | 17206 | 18832 | 43934 | 159.9 | $0.0025 |

Total cost: $0.0050 (log: /home/elena.aschmann/llm-asm-controls/results/probe_reasoning.jsonl)
