	.file	"hand_sort8.c"
	.text
	.globl	sort8
	.type	sort8, @function
sort8:
	testq	%rdi, %rdi
	jle	.L1
	movq	%rsi, %r10
	leaq	-8(,%rdi,8), %rax
	andq	$-64, %rax
	leaq	64(%rsi,%rax), %r11
	movl	$NET+152, %r9d
	jmp	.L5
	.p2align 6
.L3:
	addq	$8, %rax
	cmpq	%r9, %rax
	je	.L8
.L4:
	movslq	(%rax), %rdx
	leaq	(%r8,%rdx,8), %rsi
	movq	(%rsi), %rcx
	movslq	4(%rax), %rdx
	leaq	(%r8,%rdx,8), %rdx
	movq	(%rdx), %rdi
	cmpq	%rdi, %rcx
	jle	.L3
	movq	%rdi, (%rsi)
	movq	%rcx, (%rdx)
	jmp	.L3
.L8:
	addq	$64, %r10
	cmpq	%r11, %r10
	je	.L1
.L5:
	movq	%r10, %r8
	movl	$NET, %eax
	jmp	.L4
.L1:
	ret
	.size	sort8, .-sort8
	.section	.rodata
	.align 32
	.type	NET, @object
	.size	NET, 152
NET:
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.long	0
	.long	2
	.long	1
	.long	3
	.long	4
	.long	6
	.long	5
	.long	7
	.long	1
	.long	2
	.long	5
	.long	6
	.long	0
	.long	4
	.long	3
	.long	7
	.long	1
	.long	5
	.long	2
	.long	6
	.long	1
	.long	4
	.long	3
	.long	6
	.long	2
	.long	4
	.long	3
	.long	5
	.long	3
	.long	4
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
