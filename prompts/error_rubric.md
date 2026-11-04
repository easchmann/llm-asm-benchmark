# Failure classification rubric

Each failed attempt gets one main category, the error that alone explains the failure: if only this error
were fixed, the attempt would most likely pass (or get much further). If there are several independent errors,
the main category is the one that shows up first in the test output (smallest failing size, earliest
check); the others are secondary. Never pick a category without concrete evidence (line number, instruction,
failing size or check).

Rules assign X, A and B from the build stage, C from the driver's ABI checks and crash sites, and E from the
size sweep. The judge classifies everything else, normally into C, D, E, F or G.

## Categories and subtypes

**A. assembler/syntax** (did not assemble)
- operand_order: AT&T source/destination swapped (e.g. immediate or memory as the destination where not allowed)
- size_suffix: wrong or missing size suffix (`movl` on 64-bit registers, ambiguous memory operand size)
- invalid_operand: operand combination the instruction does not accept, wrong register class or width
- unknown_mnemonic: mnemonic that does not exist (hallucinated) or is misspelled
- intel_syntax: Intel syntax mixed in (`[rdi+8]`, `qword ptr`, registers without `%`)
- directive: unknown or malformed assembler directive
- preprocessor: a `#` comment at the start of a line is read by the C preprocessor (the .S file is
  preprocessed) and is not a valid directive
- other_syntax: anything else (stray prose or fences inside the code, unbalanced parentheses)

**B. link/symbol** (did not link, or the assembler rejected a symbol: undefined local label, `.size` of
an undefined function label, duplicate label)
- undefined_symbol: reference to a label or symbol that is not defined, or the function label misspelled
- missing_globl: function not exported
- section: code or data in the wrong section
- pie_relocation: absolute addressing of data (needs `(%rip)`), rejected when linking a PIE
- duplicate_symbol: symbol defined twice
- other_link

**C. ABI / calling convention**
- callee_saved: rbx, rbp, r12-r15 changed and not restored
- stack_pointer: rsp not restored, unbalanced push/pop, ret to a wrong address
- stack_alignment: aligned access to a stack slot that is not aligned (e.g. after an odd number of pushes)
- arg_registers: arguments read from the wrong registers (System V: integer/pointer arguments in rdi, rsi,
  rdx, rcx, r8, r9 in order of the integer arguments only; floating-point arguments in xmm0-xmm7 counted
  separately)
- return_register: result returned in the wrong register (integer in rax, double in xmm0)
- vzeroupper: missing vzeroupper after AVX code (only ever secondary: it does not change results)

**D. instruction semantics** (the intended algorithm is right, but an instruction does something other than
what the code assumes)
- shuffle_permute_blend: wrong immediate or control for shuffle, permute, blend, insert, extract, broadcast
- lane_order: assumes a 256-bit operation crosses the 128-bit lanes when it does not (vhaddpd, vunpck*,
  vshufpd, vpshufb), or wrong element order after a lane-crossing permute
- packed_scalar: scalar instruction where packed was needed or the reverse; upper elements ignored or garbage
- horizontal_reduction: wrong sum/max of the elements of a vector register
- aligned_access: aligned instruction (or legacy SSE memory operand) on an address that is not aligned
- element_size: wrong element width or type (vpaddd vs vpaddq, ps vs pd, cvt variants, imul vs pmuludq)
- addressing: wrong scale factor or byte/element confusion in an address (scale 4 for doubles, offset 1 instead of 8)
- reg_conflict: a register still holding a live value is overwritten (including argument registers before use,
  and registers implicitly written by an instruction such as rdx by mul/div or the upper half of ymm by a VEX
  128-bit op)
- flags_condition: wrong condition code (signed vs unsigned, inverted), flags clobbered between cmp and jcc
- isa: instruction outside the allowed ISA (AVX-512, zmm registers) or not supported by the CPU
- operand_order: AT&T source/destination order confused in an instruction that still assembles (a store
  written as a load, the wrong destination of a three-operand VEX instruction); inverted cmp/jcc is flags_condition
- other_semantics

**E. tails/boundaries** (main vectorised path correct, remainder or small sizes wrong)
- tail: wrong when the trip count is not a multiple of the vector width / unroll factor: missing remainder loop,
  wrong remainder bound, remainder reads or writes past the end
- small_n: wrong only for sizes smaller than one vector or one unrolled iteration (including n = 1)

**F. algorithm** (the computation itself is wrong, independent of instruction details; fails also at sizes
that are multiples of the vector width)
- loop_bounds: wrong start, end or step of a loop
- indexing: wrong index expression, transposed access, wrong array
- stride: wrong row length / leading dimension
- math: wrong formula, constant, coefficient or operation order that changes the result
- update_order: in-place vs double-buffered mix-up, wrong time-step structure, wrong dependency order
- initialization: output not initialized or accumulator not reset
- other_algorithm

**G. crash/timeout not otherwise explained**
- crash_unknown, timeout_unknown: the cause cannot be determined from the code and test output

**X. no usable code** (rule only)
- no_code: no function in the answer; incomplete: answer cut off, no ret

## Tie-breaks

- C vs D vs F for a crash: look at the crash site. Ret to an invalid address or a crash in the test program after
  return is C unless the code clearly writes out of bounds (then F, or E if only at tail sizes).
- D vs F: if the loop structure, indices and bounds would be right with the instructions doing what the author
  apparently intended, it is D. If the author's intended computation is wrong, it is F.
- Addressing: scale factor or byte/element confusion is D (addressing); using the wrong dimension for a row
  length is F (stride).
- E needs evidence that the multiple-of-width sizes pass (size sweep) or that only the last elements / guard
  regions are wrong. A failure at every size is never E.
- A timeout is F when the loop counter or bound is visibly wrong (e.g. never reaches the end condition), D when
  caused by a wrong condition code, otherwise G.
