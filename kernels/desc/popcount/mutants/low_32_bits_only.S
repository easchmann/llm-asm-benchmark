	.file	"hand_popcount.c"
	.text
	.globl	popcount
	.type	popcount, @function
popcount:
	testq	%rdi, %rdi
	jle	.L1
	movl	$0, %r8d
	movl	$0, %r10d
.L5:
	movq	%r8, %r9
	movq	(%rsi,%r8,8), %rcx
	testq	%rcx, %rcx
	je	.L6
	movq	%rcx, %rax
	.p2align 3
.L4:
	blsr	%rax, %rax
	jne	.L4
	popcntl	%ecx, %ecx	/* mutant: counts only the low 32 bits */
.L3:
	movq	%rcx, (%rdx,%r9,8)
	addq	$1, %r8
	cmpq	%r8, %rdi
	jne	.L5
.L1:
	ret
.L6:
	movq	%r10, %rcx
	jmp	.L3
	.size	popcount, .-popcount
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
