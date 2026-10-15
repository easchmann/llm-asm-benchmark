	.file	"mvt.c"
	.text
	.p2align 4
	.globl	mvt
	.type	mvt, @function
mvt:
	testq	%rdi, %rdi
	jle	.L16
	leaq	0(,%rdi,8), %r10
	pushq	%rbp
	movq	%rdi, %r11
	movq	%r9, %rdi
	pushq	%rbx
	leaq	(%rsi,%r10), %rbp
	movq	%rcx, %rbx
	.p2align 4
	.p2align 3
.L5:
	vmovsd	(%rsi), %xmm1
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L4:
	vmovsd	(%rdi,%rax,8), %xmm0
	vmulsd	(%rbx,%rax,8), %xmm0, %xmm0
	movq	%rax, %rcx
	leaq	1(%rax), %rax
	vaddsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi)
	cmpq	%rax, %r11
	jne	.L4
	addq	$8, %rsi
	addq	%r10, %rdi
	cmpq	%rsi, %rbp
	jne	.L5
	movq	%rdx, %rsi
	leaq	(%r8,%r10), %rdi
	xorl	%r11d, %r11d
	.p2align 4
	.p2align 3
.L6:
	vmovsd	(%rsi), %xmm1
	movq	%r8, %rax
	movq	%r9, %rdx
	.p2align 5
	.p2align 4
	.p2align 3
.L7:
	vmovsd	(%rdx), %xmm0
	vmulsd	(%rax), %xmm0, %xmm0
	addq	$8, %rax
	addq	%r10, %rdx
	vaddsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi)
	cmpq	%rax, %rdi
	jne	.L7
	leaq	1(%r11), %rax
	addq	$8, %rsi
	addq	$8, %r9
	cmpq	%r11, %rcx
	je	.L11
	movq	%rax, %r11
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L11:
	popq	%rbx
	popq	%rbp
	ret
.L16:
	ret
	.size	mvt, .-mvt
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
