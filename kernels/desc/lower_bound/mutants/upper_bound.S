	.file	"hand_lower_bound.c"
	.text
	.globl	lower_bound
	.type	lower_bound, @function
lower_bound:
	testq	%rdx, %rdx
	jle	.L9
	pushq	%rbx
	movq	%rdi, %r10
	movq	%rsi, %r9
	movq	%rcx, %rdi
	leaq	(%rcx,%rdx,8), %rbx
	movl	$0, %r11d
	jmp	.L3
.L13:
	leaq	1(%rax), %rdx
	jmp	.L4
.L12:
	movq	%rdx, (%r8)
	addq	$8, %r8
	addq	$8, %rdi
	cmpq	%rbx, %rdi
	je	.L1
.L3:
	movq	%r10, %rcx
	movq	%r11, %rdx
.L4:
	cmpq	%rdx, %rcx
	jle	.L12
	movq	%rcx, %rsi
	subq	%rdx, %rsi
	movq	%rsi, %rax
	shrq	$63, %rax
	addq	%rsi, %rax
	sarq	%rax
	addq	%rdx, %rax
	movq	(%rdi), %rsi
	cmpq	%rsi, (%r9,%rax,8)
	jle	.L13	/* mutant: <= instead of <, returns the upper bound */
	movq	%rax, %rcx
	jmp	.L4
.L1:
	popq	%rbx
	ret
.L9:
	ret
	.size	lower_bound, .-lower_bound
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
