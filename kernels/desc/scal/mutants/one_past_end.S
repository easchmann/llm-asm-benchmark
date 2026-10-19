	.file	"hand_scal.c"
	.text
	.globl	scal
	.type	scal, @function
scal:
	testq	%rdi, %rdi
	jle	.L1
	movq	%rsi, %rax
	leaq	8(%rsi,%rdi,8), %rdx	/* mutant: loop runs one element past the end of x */
	.p2align 5
.L3:
	vmulsd	(%rax), %xmm0, %xmm1
	vmovsd	%xmm1, (%rax)
	addq	$8, %rax
	cmpq	%rdx, %rax
	jne	.L3
.L1:
	ret
	.size	scal, .-scal
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
