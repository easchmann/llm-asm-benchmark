	.file	"hand_prefix_sum.c"
	.text
	.globl	prefix_sum
	.type	prefix_sum, @function
prefix_sum:
	testq	%rdi, %rdi
	jle	.L1
	movl	$0, %eax
	movl	$0, %r8d
	.p2align 5
.L3:
	movq	%r8, %rcx
	addq	(%rsi,%rax,8), %rcx
	movq	%r8, (%rdx,%rax,8)	/* mutant: stores the sum before adding a[i] (exclusive scan) */
	movq	%rcx, %r8	/* mutant: stores the sum before adding a[i] (exclusive scan) */
	addq	$1, %rax
	cmpq	%rax, %rdi
	jne	.L3
.L1:
	ret
	.size	prefix_sum, .-prefix_sum
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
