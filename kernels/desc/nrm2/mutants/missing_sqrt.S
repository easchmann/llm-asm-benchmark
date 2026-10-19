	.file	"hand_nrm2.c"
	.text
	.globl	nrm2
	.type	nrm2, @function
nrm2:
	testq	%rdi, %rdi
	jle	.L4
	movq	%rsi, %rax
	leaq	(%rsi,%rdi,8), %rdx
	vxorpd	%xmm0, %xmm0, %xmm0
	.p2align 5
.L3:
	vmovsd	(%rax), %xmm1
	vmulsd	%xmm1, %xmm1, %xmm1
	vaddsd	%xmm1, %xmm0, %xmm0
	addq	$8, %rax
	cmpq	%rdx, %rax
	jne	.L3
.L2:
	nop	/* mutant: square root removed */
	ret
.L4:
	vxorpd	%xmm0, %xmm0, %xmm0
	jmp	.L2
	.size	nrm2, .-nrm2
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
