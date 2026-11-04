## Variants

| run | model | base model | underlying model | cells | no answer | reasoning tokens per call | reasoning |
|---|---|---|---|---|---|---|---|
| desc_claude | claude-haiku-4-5-20251001@thinking-off | claude-haiku-4-5-20251001 | claude-haiku-4-5-20251001 | 480 | 0 | 0 | off |
| desc_claude | claude-haiku-4-5-20251001@thinking-on | claude-haiku-4-5-20251001 | claude-haiku-4-5-20251001 | 480 | 0 | 12905 | on |
| desc_claude | claude-haiku-5-5@thinking-on | claude-haiku-5-5 | claude-haiku-5-5 | 480 | 0 | 3881 | on |
| desc_claude | claude-opus-5-5@thinking-on | claude-opus-5-5 | claude-opus-5-5 | 480 | 0 | 1059 | on |
| desc_claude | claude-opus-5@thinking-off | claude-opus-5 | claude-opus-5 | 480 | 0 | 0 | off |
| desc_claude | claude-opus-5@thinking-on | claude-opus-5 | claude-opus-5 | 480 | 0 | 6604 | on |
| desc_claude | claude-sonnet-5-5@thinking-on | claude-sonnet-5-5 | claude-sonnet-5-5 | 480 | 0 | 825 | on |
| desc_claude | claude-sonnet-5@thinking-off | claude-sonnet-5 | claude-sonnet-5 | 480 | 0 | 0 | off |
| desc_claude | claude-sonnet-5@thinking-on | claude-sonnet-5 | claude-sonnet-5 | 480 | 0 | 6295 | on |

## Pass rate, on - off (95% CI, bootstrap over kernels)

| off | on | level | cells | first try off | first try on | on - off | after repair off | after repair on | on - off |
|---|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | claude-haiku-4-5-20251001@thinking-on (desc_claude) | all | 480 | 25% | 54% | +29 pts [+19, +38] | 66% | 91% | +25 pts [+19, +32] |
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | claude-haiku-4-5-20251001@thinking-on (desc_claude) | L0 | 120 | 24% | 56% | +32 pts [+16, +48] | 70% | 93% | +23 pts [+13, +34] |
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | claude-haiku-4-5-20251001@thinking-on (desc_claude) | L1 | 120 | 20% | 58% | +38 pts [+25, +52] | 67% | 92% | +26 pts [+19, +33] |
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | claude-haiku-4-5-20251001@thinking-on (desc_claude) | L2 | 120 | 29% | 53% | +24 pts [+11, +37] | 62% | 93% | +31 pts [+22, +39] |
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | claude-haiku-4-5-20251001@thinking-on (desc_claude) | L3 | 120 | 27% | 47% | +20 pts [+6, +33] | 65% | 87% | +22 pts [+8, +34] |
| claude-opus-5@thinking-off (desc_claude) | claude-opus-5@thinking-on (desc_claude) | all | 480 | 83% | 99% | +15 pts [+9, +24] | 99% | 100% | +1 pts [+0, +2] |
| claude-opus-5@thinking-off (desc_claude) | claude-opus-5@thinking-on (desc_claude) | L0 | 120 | 91% | 100% | +9 pts [+5, +14] | 100% | 100% | +0 pts [+0, +0] |
| claude-opus-5@thinking-off (desc_claude) | claude-opus-5@thinking-on (desc_claude) | L1 | 120 | 82% | 98% | +16 pts [+7, +25] | 99% | 100% | +1 pts [+0, +3] |
| claude-opus-5@thinking-off (desc_claude) | claude-opus-5@thinking-on (desc_claude) | L2 | 120 | 79% | 98% | +18 pts [+8, +30] | 99% | 100% | +1 pts [+0, +3] |
| claude-opus-5@thinking-off (desc_claude) | claude-opus-5@thinking-on (desc_claude) | L3 | 120 | 81% | 99% | +18 pts [+8, +30] | 99% | 100% | +1 pts [+0, +3] |
| claude-sonnet-5@thinking-off (desc_claude) | claude-sonnet-5@thinking-on (desc_claude) | all | 480 | 62% | 91% | +28 pts [+19, +39] | 97% | 100% | +3 pts [+1, +5] |
| claude-sonnet-5@thinking-off (desc_claude) | claude-sonnet-5@thinking-on (desc_claude) | L0 | 120 | 62% | 89% | +28 pts [+16, +42] | 97% | 100% | +3 pts [+1, +7] |
| claude-sonnet-5@thinking-off (desc_claude) | claude-sonnet-5@thinking-on (desc_claude) | L1 | 120 | 63% | 92% | +28 pts [+15, +42] | 98% | 100% | +2 pts [+0, +4] |
| claude-sonnet-5@thinking-off (desc_claude) | claude-sonnet-5@thinking-on (desc_claude) | L2 | 120 | 64% | 92% | +27 pts [+12, +43] | 96% | 100% | +4 pts [+1, +7] |
| claude-sonnet-5@thinking-off (desc_claude) | claude-sonnet-5@thinking-on (desc_claude) | L3 | 120 | 60% | 90% | +30 pts [+18, +43] | 98% | 100% | +3 pts [+0, +5] |

## First try: pass and failure types (share of cells)

| variant | reasoning | cells | pass | assembler error | crash or timeout | ABI or memory violation | wrong values, larger sizes only | wrong values, all sizes | other (link, unfinished) |
|---|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | off | 480 | 25% | 31% | 11% | 11% | 5% | 14% | 1% |
| claude-haiku-4-5-20251001@thinking-on (desc_claude) | on | 480 | 54% | 26% | 4% | 6% | 3% | 8% | 0% |
| claude-opus-5@thinking-off (desc_claude) | off | 480 | 83% | 2% | 1% | 10% | 2% | 1% | 0% |
| claude-opus-5@thinking-on (desc_claude) | on | 480 | 99% | 0% | 0% | 0% | 0% | 0% | 0% |
| claude-sonnet-5@thinking-off (desc_claude) | off | 480 | 62% | 12% | 4% | 18% | 3% | 1% | 0% |
| claude-sonnet-5@thinking-on (desc_claude) | on | 480 | 91% | 3% | 0% | 5% | 0% | 1% | 0% |

## After repair: pass and failure types (share of cells)

| variant | reasoning | cells | pass | assembler error | crash or timeout | ABI or memory violation | wrong values, larger sizes only | wrong values, all sizes | other (link, unfinished) |
|---|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | off | 480 | 66% | 2% | 7% | 8% | 2% | 15% | 0% |
| claude-haiku-4-5-20251001@thinking-on (desc_claude) | on | 480 | 91% | 0% | 1% | 0% | 1% | 6% | 0% |
| claude-opus-5@thinking-off (desc_claude) | off | 480 | 99% | 0% | 0% | 0% | 0% | 0% | 0% |
| claude-opus-5@thinking-on (desc_claude) | on | 480 | 100% | 0% | 0% | 0% | 0% | 0% | 0% |
| claude-sonnet-5@thinking-off (desc_claude) | off | 480 | 97% | 0% | 1% | 1% | 1% | 0% | 0% |
| claude-sonnet-5@thinking-on (desc_claude) | on | 480 | 100% | 0% | 0% | 0% | 0% | 0% | 0% |

## Tokens, time and cost per cell (all rounds)

| variant | reasoning | cells | calls per cell | reasoning tokens | completion tokens | s per cell | slowest call s | calls >= 600 s | cost per cell | cost per solved cell |
|---|---|---|---|---|---|---|---|---|---|---|
| claude-haiku-4-5-20251001@thinking-off (desc_claude) | off | 480 | 2.42 | 0 | 4017 | 26 | 41 | 0 | $0.0246 | $0.0372 |
| claude-haiku-4-5-20251001@thinking-on (desc_claude) | on | 480 | 1.60 | 20675 | 22389 | 137 | 591 | 0 | $0.1140 | $0.1247 |
| claude-opus-5@thinking-off (desc_claude) | off | 480 | 1.19 | 0 | 5117 | 51 | 613 | 1 | $0.1466 | $0.1475 |
| claude-opus-5@thinking-on (desc_claude) | on | 480 | 1.01 | 6700 | 9294 | 102 | 711 | 1 | $0.2426 | $0.2426 |
| claude-sonnet-5@thinking-off (desc_claude) | off | 480 | 1.46 | 0 | 3690 | 30 | 237 | 0 | $0.0463 | $0.0477 |
| claude-sonnet-5@thinking-on (desc_claude) | on | 480 | 1.10 | 6938 | 8295 | 78 | 620 | 1 | $0.0869 | $0.0869 |

