	.file	"hand_histogram.c"
	.text
	.globl	histogram
	.type	histogram, @function
histogram:
	testq	%rdx, %rdx
	jle	.L2
	movq	%rcx, %rax
	leaq	(%rcx,%rdx,8), %rdx
	.p2align 4
.L3:
	nop	/* mutant: hist[b] = 0 removed */
	addq	$8, %rax
	cmpq	%rdx, %rax
	jne	.L3
.L2:
	testq	%rdi, %rdi
	jle	.L1
	movq	%rsi, %rax
	leaq	(%rsi,%rdi,8), %rsi
	.p2align 5
.L5:
	movq	(%rax), %rdx
	addq	$1, (%rcx,%rdx,8)
	addq	$8, %rax
	cmpq	%rsi, %rax
	jne	.L5
.L1:
	ret
	.size	histogram, .-histogram
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
