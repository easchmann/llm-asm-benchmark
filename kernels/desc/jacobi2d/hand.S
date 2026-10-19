	.file	"hand_jacobi2d.c"
	.text
	.globl	jacobi2d
	.type	jacobi2d, @function
jacobi2d:
	testq	%rdi, %rdi
	jle	.L14
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, %r10
	movq	%rsi, %r9
	movq	%rdx, %rbx
	movq	%rcx, %r15
	leaq	0(,%rsi,8), %rdi
	movq	%rsi, %r11
	salq	$4, %r11
	addq	%rdx, %r11
	leaq	(%rcx,%rdi), %rbp
	movq	%rsi, %r8
	negq	%r8
	salq	$3, %r8
	leaq	0(%rbp,%rdi), %rax
	movq	%rax, -16(%rsp)
	leaq	(%r11,%r8), %rax
	movq	%rax, -8(%rsp)
	movl	$0, %edx
	leaq	-1(%rsi), %rcx
	vmovsd	.LC0(%rip), %xmm2
	vmovapd	%xmm2, %xmm1
	movq	%rdx, -24(%rsp)
	movq	%r11, %rsi
	jmp	.L3
.L9:
	addq	$1, %r14
	leaq	(%r11,%r8), %rdx
	movl	$1, %eax
	.p2align 6
.L4:
	vmovsd	8(%rdx), %xmm0
	vaddsd	(%rdx), %xmm0, %xmm0
	vaddsd	16(%rdx), %xmm0, %xmm0
	vaddsd	(%r11,%rax,8), %xmm0, %xmm0
	vaddsd	0(%r13,%rax,8), %xmm0, %xmm0
	vmulsd	%xmm2, %xmm0, %xmm0
	vmovsd	%xmm0, (%r12,%rax,8)
	addq	$1, %rax
	addq	$8, %rdx
	cmpq	%rcx, %rax
	jne	.L4
	addq	%rdi, %r11
	addq	%rdi, %r13
	addq	%rdi, %r12
	cmpq	%rcx, %r14
	jne	.L9
	movq	%r15, %r13
	movq	-8(%rsp), %r12
	movq	-16(%rsp), %r11
	movl	$1, %r14d
.L6:
	addq	$1, %r14
	leaq	(%r11,%r8), %rdx
	movl	$1, %eax
	.p2align 6
.L7:
	vmovsd	8(%rdx), %xmm0
	vaddsd	(%rdx), %xmm0, %xmm0
	vaddsd	16(%rdx), %xmm0, %xmm0
	vaddsd	(%r11,%rax,8), %xmm0, %xmm0
	vaddsd	0(%r13,%rax,8), %xmm0, %xmm0
	vmulsd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r12,%rax,8)
	addq	$1, %rax
	addq	$8, %rdx
	cmpq	%rcx, %rax
	jne	.L7
	addq	%rdi, %r11
	addq	%rdi, %r13
	addq	%rdi, %r12
	cmpq	%rcx, %r14
	jne	.L6
.L8:
	addq	$1, -24(%rsp)
	movq	-24(%rsp), %rax
	cmpq	%rax, %r10
	je	.L1
.L3:
	cmpq	$2, %r9
	jle	.L8
	movq	%rbx, %r13
	movq	%rbp, %r12
	movq	%rsi, %r11
	movl	$1, %r14d
	jmp	.L9
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L14:
	ret
	.size	jacobi2d, .-jacobi2d
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	-1717986918
	.long	1070176665
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
