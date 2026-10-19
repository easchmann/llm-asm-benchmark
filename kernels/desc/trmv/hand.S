	.file	"hand_trmv.c"
	.text
	.globl	trmv
	.type	trmv, @function
trmv:
	movq	%rdi, %r9
	testq	%rdi, %rdi
	jle	.L1
	leaq	0(,%rdi,8), %r10
	movl	$0, %r8d
.L4:
	movq	%rcx, %rdi
	movq	$0x000000000, (%rcx)
	movl	$0, %eax
	.p2align 5
.L3:
	vmovsd	(%rsi,%rax,8), %xmm0
	vmulsd	(%rdx,%rax,8), %xmm0, %xmm0
	vaddsd	(%rdi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdi)
	addq	$1, %rax
	cmpq	%r8, %rax
	jle	.L3
	addq	$1, %r8
	addq	$8, %rcx
	addq	%r10, %rsi
	cmpq	%r8, %r9
	jne	.L4
.L1:
	ret
	.size	trmv, .-trmv
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
