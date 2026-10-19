	.file	"axpy.c"
	.text
	.p2align 4
	.globl	axpy
	.type	axpy, @function
axpy:
	movq	$0, %rbx	/* mutant: callee-saved rbx overwritten without being saved */
	testq	%rdi, %rdi
	jle	.L5
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L3:
	vmovsd	(%rsi,%rax,8), %xmm1
	vfmadd213sd	(%rdx,%rax,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdx,%rax,8)
	addq	$1, %rax
	cmpq	%rax, %rdi
	jne	.L3
.L5:
	ret
	.size	axpy, .-axpy
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
