# Plan variants

Four ablations of the teacher plans in `plans/claude-opus-5-5`, used with `--plan-from plans/<variant>`. Each
variant has the same layout (`<kernel>/<level>.txt`), so `gen_desc.py` gives it to the model through the unchanged
template `prompts/desc_from_plan_x86_64.txt` ("Before writing code, you made the implementation plan below ...").

Written by hand (Claude, in one session) from the teacher plans; `scripts/plan_variants.py build` writes the
per-level files from the sources listed below, and `scripts/plan_variants.py check` checks them.
20 of 24 kernels have the same teacher plan at every level, so each distinct teacher text was rewritten once:
heat3d has three (L0, L2, L1 = L3), jacobi2d two (L0-L2, L3), lower_bound two (L0, L1-L3), sort8 two (L0, L1-L3).

## Rules for all variants

- Use only content from the teacher plan (except the cheatsheet). Do not add facts, hints or corrections.
- Keep the teacher's wording wherever a sentence survives; delete rather than paraphrase.
- Differences between levels are kept exactly where the teacher plan differs between levels.

## algo_only (source: `plans/algo_only/PLANS.txt`)

Keep: loop structure and order, loop bounds in index terms, blocking and fusion (e.g. both products in one pass in
mvt), data flow and in-place reasoning, initialisation of outputs, the maths (formulas, order of additions, constants
such as 0.2 or the popcount masks), algorithmic edge cases (borders never written, empty rows, signed values,
which elements are never read).

Remove: register names and register roles, instruction names, System V argument passing, callee-saved registers,
stack offsets, vzeroupper, AVX/SSE/FMA/VEX, vector width, lanes, broadcasts, horizontal reductions, alignment,
vector part and scalar tail, byte offsets and strides (addresses become array indices), .rodata constants,
conditional moves and flags. Summation-order notes that only concern SIMD lanes are removed; order notes that
apply to the plain loop (e.g. increasing k) are kept. "Logical shift" becomes "unsigned shift"; "keep the low 64
bits of the product" becomes "mod 2^64".

Sections: "Algorithm", "Edge cases".

## lowlevel_only (source: `plans/lowlevel_only/PLANS.txt`)

Keep: argument registers and stack slots, return register, callee-saved registers with push/pop and the shifted
stack offsets, what a register holds (named by role: "column index", "row pointer", "vector accumulator",
"n & -4, the end of the vector part"), constants in registers and .rodata, element addresses and strides in bytes,
vector width and instruction set (AVX 256-bit, no FMA, VEX scalar forms), how a vector accumulator is reduced,
how tails are handled (vector part up to n & -4, scalar rest), memory bounds of loads and stores, alignment,
vzeroupper, conditional-move technique, POPCNT not allowed, signed vs unsigned comparisons and shifts.

Remove: loop order and nesting, loop bounds other than the vector/scalar split, what is computed (formulas,
products, sums, stencil weights applied, search or sort algorithm, network order), order of phases, pointer
swapping between time steps, initialisation of outputs, algorithmic edge cases. Register roles that only make
sense as a loop step ("rdi counts rows down") are phrased as an option ("rdi can count rows").

Sections: "Registers (System V)", "Memory and tails" (or "Memory ..." for scalar kernels), "Vector instructions".

## cheatsheet (source: `plans/cheatsheet/CHEATSHEET.txt`)

One kernel-independent page (about 575 words), the same text for every kernel and level: AT&T syntax pitfalls,
System V ABI, the SSE2/AVX/AVX2 instructions used by these kernels, a horizontal reduction, scalar and masked
tails, common mistakes. It is new content (not taken from the teacher plans) and contains no algorithm.
Note: the template still calls it "the implementation plan below".

## seeded_error (source: `plans/seeded_error/seeds.json`, record: `plans/seeded_error/ERRORS.md`)

The teacher plan with exactly one plausible algorithmic error per kernel, the same error at every level (for
kernels whose teacher text differs between levels the replacement is applied to each distinct text). Rules:

- The error changes what is computed, not how it is encoded in x86-64 (no register, ABI or SIMD errors).
- The rest of the plan is made consistent with the error (edge cases, register comments), so the plan is
  coherent and the error is only visible against the task description.
- The error must not cause out-of-bounds accesses, so a model that follows the plan produces wrong values,
  not crashes.
- `<kernel>/seeded_ref.c` is the C reference with the same error; `plan_variants.py check` verifies the
  tests fail on it, and `scripts/plan_ablation.py` uses it to classify answers that reproduce the error.
