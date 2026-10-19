	.file	"hand_jacobi1d.c"
	.text
	.globl	jacobi1d
	.type	jacobi1d, @function
jacobi1d:
	movq	%rsi, %r8
	movl	$0, %r10d
	testq	%rdi, %rdi
	jle	.L1
	leaq	-1(%rsi), %r9
	vmovsd	.LC0(%rip), %xmm2
	vmovapd	%xmm2, %xmm1
	jmp	.L2
	.p2align 6
.L4:
	vmovsd	-8(%rdx,%rax,8), %xmm0
	vaddsd	(%rdx,%rax,8), %xmm0, %xmm0
	vaddsd	8(%rdx,%rax,8), %xmm0, %xmm0
	vdivsd	%xmm2, %xmm0, %xmm0
	vmovsd	%xmm0, (%rcx,%rax,8)
	movq	%rax, %rsi
	addq	$1, %rax
	cmpq	%r9, %rax
	jne	.L4
	movl	$1, %eax
	.p2align 6
.L5:
	movq	%rax, %r11
	vmovsd	-8(%rcx,%rax,8), %xmm0
	vaddsd	(%rcx,%rax,8), %xmm0, %xmm0
	vaddsd	8(%rcx,%rax,8), %xmm0, %xmm0
	vdivsd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%rax,8)
	addq	$1, %rax
	cmpq	%r11, %rsi
	jne	.L5
.L7:
	addq	$1, %r10
	cmpq	%r10, %rdi
	je	.L1
.L2:
	movl	$0, %eax	/* mutant: first sweep starts at i = 0, overwriting B[0] */
	cmpq	$2, %r8
	jg	.L4
	jmp	.L7
.L1:
	ret
	.size	jacobi1d, .-jacobi1d
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	0
	.long	1074266112
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
