	.file	"hand_3mm.c"
	.text
	.globl	mm3
	.type	mm3, @function
mm3:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, -40(%rsp)
	movq	%rsi, %r11
	movq	%rdx, %rbx
	movq	%rcx, %rax
	movq	%r8, %rdx
	movq	%r9, %r15
	testq	%rdi, %rdi
	jle	.L2
	leaq	0(,%rbx,8), %r14
	movq	%r14, %rsi
	addq	56(%rsp), %rsi
	leaq	0(,%r11,8), %rdi
	movl	$0, %r12d
	movl	$0, %r13d
	movl	$0, %ecx
	movq	%rax, -16(%rsp)
	movq	%r8, -8(%rsp)
	movq	%r9, %rdx
	movq	56(%rsp), %r8
	jmp	.L3
.L8:
	leaq	(%rdx,%r13,8), %r9
	movq	64(%rsp), %rbp
	leaq	(%r8,%r12,8), %r15
	movl	$0, %r10d
	movq	%rcx, -32(%rsp)
	movq	%r13, -24(%rsp)
.L6:
	movq	%r9, %r13
	nop	/* mutant: E[i][j] = 0 removed */
	testq	%rbx, %rbx
	jle	.L4
	movq	%rbp, %rcx
	movq	%r15, %rax
	.p2align 5
.L5:
	vmovsd	(%rax), %xmm0
	vmulsd	(%rcx), %xmm0, %xmm0
	vaddsd	0(%r13), %xmm0, %xmm0
	vmovsd	%xmm0, 0(%r13)
	addq	$8, %rax
	addq	%rdi, %rcx
	cmpq	%rsi, %rax
	jne	.L5
.L4:
	addq	$1, %r10
	addq	$8, %r9
	addq	$8, %rbp
	cmpq	%r10, %r11
	jne	.L6
	movq	-32(%rsp), %rcx
	movq	-24(%rsp), %r13
.L9:
	addq	$1, %rcx
	addq	%r11, %r13
	addq	%rbx, %r12
	addq	%r14, %rsi
	cmpq	%rcx, -40(%rsp)
	je	.L7
.L3:
	testq	%r11, %r11
	jg	.L8
	jmp	.L9
.L7:
	movq	-16(%rsp), %rax
	movq	%rdx, %r15
	movq	-8(%rsp), %rdx
	testq	%r11, %r11
	jle	.L10
.L25:
	leaq	0(,%rdx,8), %r14
	movq	%r14, %r8
	addq	80(%rsp), %r8
	leaq	0(,%rax,8), %r9
	movl	$0, %r13d
	movl	$0, %edi
	movl	$0, %esi
	movq	%r15, -16(%rsp)
	movq	80(%rsp), %r10
	jmp	.L11
.L16:
	movq	72(%rsp), %rbx
	leaq	(%rbx,%rdi,8), %rbx
	movq	88(%rsp), %r12
	leaq	(%r10,%r13,8), %r15
	movl	$0, %ebp
	movq	%rsi, -32(%rsp)
	movq	%rdi, -24(%rsp)
.L14:
	movq	%rbx, %rdi
	movq	$0x000000000, (%rbx)
	testq	%rdx, %rdx
	jle	.L12
	movq	%r12, %rsi
	movq	%r15, %rcx
	.p2align 5
.L13:
	vmovsd	(%rcx), %xmm0
	vmulsd	(%rsi), %xmm0, %xmm0
	vaddsd	(%rdi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdi)
	addq	$8, %rcx
	addq	%r9, %rsi
	cmpq	%r8, %rcx
	jne	.L13
.L12:
	addq	$1, %rbp
	addq	$8, %rbx
	addq	$8, %r12
	cmpq	%rbp, %rax
	jne	.L14
	movq	-32(%rsp), %rsi
	movq	-24(%rsp), %rdi
.L17:
	addq	$1, %rsi
	addq	%rax, %rdi
	addq	%rdx, %r13
	addq	%r14, %r8
	cmpq	%rsi, %r11
	jle	.L15
.L11:
	testq	%rax, %rax
	jg	.L16
	jmp	.L17
.L15:
	movq	-16(%rsp), %r15
	cmpq	$0, -40(%rsp)
	jle	.L1
.L10:
	leaq	0(,%r11,8), %r13
	leaq	(%r15,%r13), %rdi
	leaq	0(,%rax,8), %r8
	movl	$0, %r12d
	movl	$0, %esi
	movl	$0, %ecx
	movq	%r15, %r9
	movq	96(%rsp), %r10
	jmp	.L19
.L23:
	leaq	(%r10,%rsi,8), %rbx
	movq	72(%rsp), %r14
	leaq	(%r9,%r12,8), %r15
	movl	$0, %ebp
	movq	%rcx, -32(%rsp)
	movq	%rsi, -24(%rsp)
.L22:
	movq	%rbx, %rsi
	movq	$0x000000000, (%rbx)
	testq	%r11, %r11
	jle	.L20
	movq	%r14, %rcx
	movq	%r15, %rdx
	.p2align 5
.L21:
	vmovsd	(%rdx), %xmm0
	vmulsd	(%rcx), %xmm0, %xmm0
	vaddsd	(%rsi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi)
	addq	$8, %rdx
	addq	%r8, %rcx
	cmpq	%rdi, %rdx
	jne	.L21
.L20:
	addq	$1, %rbp
	addq	$8, %rbx
	addq	$8, %r14
	cmpq	%rbp, %rax
	jne	.L22
	movq	-32(%rsp), %rcx
	movq	-24(%rsp), %rsi
.L24:
	addq	$1, %rcx
	addq	%rax, %rsi
	addq	%r11, %r12
	addq	%r13, %rdi
	cmpq	%rcx, -40(%rsp)
	je	.L1
.L19:
	testq	%rax, %rax
	jg	.L23
	jmp	.L24
.L2:
	testq	%rsi, %rsi
	jg	.L25
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.size	mm3, .-mm3
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
