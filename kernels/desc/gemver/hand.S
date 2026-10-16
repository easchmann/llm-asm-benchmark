	.file	"hand_gemver.c"
	.text
	.globl	gemver
	.type	gemver, @function
gemver:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	64(%rsp), %r11
	movq	72(%rsp), %r14
	movq	80(%rsp), %r15
	testq	%rdi, %rdi
	jle	.L1
	movq	%rdi, %rbx
	vmovapd	%xmm0, %xmm2
	movq	%rsi, %r12
	movq	%rcx, %rbp
	movq	%r9, %r13
	leaq	0(,%rdi,8), %r10
	movq	%rsi, %r9
	movq	%rdx, %rsi
	movq	%r8, %rdi
	leaq	(%rdx,%r10), %r8
	movq	%r12, %rdx
.L3:
	movl	$0, %eax
	.p2align 6
.L4:
	vmovsd	(%rsi), %xmm0
	vmulsd	0(%rbp,%rax,8), %xmm0, %xmm0
	vaddsd	(%rdx,%rax,8), %xmm0, %xmm0
	vmovsd	(%rdi), %xmm4
	vmulsd	0(%r13,%rax,8), %xmm4, %xmm3
	vaddsd	%xmm3, %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%rax,8)
	movq	%rax, %rcx
	addq	$1, %rax
	cmpq	%rax, %rbx
	jne	.L4
	addq	%r10, %rdx
	addq	$8, %rsi
	addq	$8, %rdi
	cmpq	%r8, %rsi
	jne	.L3
	movq	%r11, %rsi
	leaq	(%r14,%r10), %rdi
	movl	$0, %r8d
.L6:
	movq	%r14, %rax
	movq	%r12, %rdx
	.p2align 5
.L7:
	vmulsd	(%rdx), %xmm1, %xmm0
	vmulsd	(%rax), %xmm0, %xmm0
	vaddsd	(%rsi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi)
	addq	%r10, %rdx
	addq	$8, %rax
	cmpq	%rdi, %rax
	jne	.L7
	leaq	1(%r8), %rax
	addq	$8, %r12
	addq	$8, %rsi
	cmpq	%r8, %rcx
	je	.L11
	movq	%rax, %r8
	jmp	.L6
.L11:
	movl	$0, %eax
	.p2align 5
.L8:
	movq	%rax, %rdx
	vmovsd	(%r11,%rax,8), %xmm0
	vaddsd	(%r15,%rax,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%r11,%rax,8)
	addq	$1, %rax
	cmpq	%rdx, %rcx
	jne	.L8
	movq	56(%rsp), %rdx
	movq	%r10, %rdi
	addq	%rdx, %rdi
.L9:
	movl	$0, %eax
	.p2align 5
.L10:
	movq	%rax, %rsi
	vmulsd	(%r9,%rax,8), %xmm2, %xmm0
	vmulsd	(%r11,%rax,8), %xmm0, %xmm0
	vaddsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx)
	addq	$1, %rax
	cmpq	%rsi, %rcx
	jne	.L10
	addq	$8, %rdx
	addq	%r10, %r9
	cmpq	%rdi, %rdx
	jne	.L9
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.size	gemver, .-gemver
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
