	.file	"hand_trmm.c"
	.text
	.globl	trmm
	.type	trmm, @function
trmm:
	testq	%rdi, %rdi
	jle	.L12
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rsi, %rbx
	vmovapd	%xmm0, %xmm1
	movq	%rdx, %r12
	movq	%rcx, %r14
	leaq	8(,%rdi,8), %r13
	leaq	0(,%rdi,8), %r8
	addq	%r8, %r12
	leaq	0(,%rsi,8), %r9
	movq	%rsi, %rcx
	movl	$0, %eax
	movl	$1, %r11d
	movq	%rax, %rdx
	movq	%r14, %r10
	jmp	.L3
.L7:
	leaq	(%r10,%rdx,8), %rbp
	movq	%rcx, %rax
	subq	%rdx, %rax
	salq	$3, %rax
	movl	$0, %r14d
	movq	%rdx, -8(%rsp)
.L6:
	movq	%r11, %rdx
	cmpq	%r11, %rdi
	jle	.L4
	leaq	0(%rbp,%rax), %r15
	movq	%r12, %rsi
	.p2align 6
.L5:
	vmovsd	(%rsi), %xmm0
	vmulsd	(%r15), %xmm0, %xmm0
	vaddsd	0(%rbp), %xmm0, %xmm0
	vmovsd	%xmm0, 0(%rbp)
	addq	$1, %rdx
	addq	%r8, %rsi
	addq	%r9, %r15
	cmpq	%rdx, %rdi
	jne	.L5
.L4:
	vmovsd	0(%rbp), %xmm0	/* mutant: B not scaled by alpha */
	vmovsd	%xmm0, 0(%rbp)
	addq	$1, %r14
	addq	$8, %rbp
	cmpq	%r14, %rbx
	jne	.L6
	movq	-8(%rsp), %rdx
.L8:
	leaq	1(%r11), %rax
	addq	%rbx, %rdx
	addq	%r13, %r12
	addq	%rbx, %rcx
	cmpq	%rdi, %r11
	je	.L1
	movq	%rax, %r11
.L3:
	testq	%rbx, %rbx
	jg	.L7
	jmp	.L8
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L12:
	ret
	.size	trmm, .-trmm
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
