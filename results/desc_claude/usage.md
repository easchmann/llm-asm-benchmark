## Thinking, per call

| variant | answered by | effort | calls | calls with thinking tokens | calls with thinking blocks | thinking tokens per call | max | finish reasons |
|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off | claude-haiku-4-5-20251001 | none | 1164 | 0 | 0 | 0 | 0 | end_turn 1164 |
| claude-haiku-4-5-20251001@thinking-on | claude-haiku-4-5-20251001 | none | 769 | 769 | 769 | 12905 | 31666 | end_turn 769 |
| claude-haiku-5-5@thinking-on | claude-haiku-5-5 | medium | 498 | 498 | 498 | 3881 | 18728 | end_turn 498 |
| claude-opus-5-5@thinking-on | claude-opus-5-5 | medium | 489 | 485 | 485 | 1059 | 4531 | end_turn 489 |
| claude-opus-5@thinking-off | claude-opus-5 | high | 573 | 0 | 0 | 0 | 0 | end_turn 573 |
| claude-opus-5@thinking-on | claude-opus-5 | high | 487 | 487 | 487 | 6604 | 46058 | end_turn 487 |
| claude-sonnet-5-5@thinking-on | claude-sonnet-5-5 | medium | 499 | 348 | 348 | 825 | 5913 | end_turn 499 |
| claude-sonnet-5@thinking-off | claude-sonnet-5 | high | 702 | 0 | 0 | 0 | 0 | end_turn 702 |
| claude-sonnet-5@thinking-on | claude-sonnet-5 | high | 529 | 486 | 486 | 6295 | 41369 | end_turn 529 |

## Cost per cell, and extrapolated to 480 cells per variant

| variant | cells | pass first try | pass after repair | no answer | calls per cell | output tokens per cell | s per call | slowest call s | s per cell | notional cost per cell | 5-hour window per cell | 7-day window per cell | time for 480 cells | 5-hour windows for 480 | 7-day window for 480 | minutes waited on limits |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off | 480 | 120 | 317 | 0 | 2.42 | 4017 | 11 | 41 | 26 | $0.025 | 0.04% | 0.01% | 3.4 h | 0.2 | 4% | 0 |
| claude-haiku-4-5-20251001@thinking-on | 480 | 257 | 439 | 0 | 1.60 | 22389 | 86 | 591 | 137 | $0.114 | 0.32% | 0.05% | 18.3 h | 1.5 | 22% | 0 |
| claude-haiku-5-5@thinking-on | 480 | 462 | 479 | 0 | 1.04 | 5492 | 23 | 94 | 24 | $0.003 | 0.01% | 0.00% | 3.2 h | 0.1 | 1% | 0 |
| claude-opus-5-5@thinking-on | 480 | 472 | 480 | 0 | 1.02 | 2626 | 23 | 78 | 23 | $0.061 | 0.03% | 0.01% | 3.1 h | 0.1 | 3% | 0 |
| claude-opus-5@thinking-off | 480 | 399 | 477 | 0 | 1.19 | 5117 | 43 | 613 | 51 | $0.147 | 0.10% | 0.01% | 6.8 h | 0.5 | 7% | 0 |
| claude-opus-5@thinking-on | 480 | 473 | 480 | 0 | 1.01 | 9294 | 100 | 711 | 102 | $0.243 | 0.24% | 0.04% | 13.6 h | 1.1 | 17% | 0 |
| claude-sonnet-5-5@thinking-on | 480 | 464 | 476 | 0 | 1.04 | 2007 | 14 | 57 | 15 | $0.024 | 0.04% | 0.01% | 2.0 h | 0.2 | 3% | 0 |
| claude-sonnet-5@thinking-off | 480 | 299 | 466 | 0 | 1.46 | 3690 | 20 | 237 | 30 | $0.046 | 0.07% | 0.01% | 4.0 h | 0.3 | 6% | 0 |
| claude-sonnet-5@thinking-on | 480 | 435 | 480 | 0 | 1.10 | 8295 | 71 | 620 | 78 | $0.087 | 0.19% | 0.03% | 10.5 h | 0.9 | 12% | 0 |

