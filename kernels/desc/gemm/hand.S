	.file	"hand_gemm.c"
	.text
	.globl	gemm
	.type	gemm, @function
gemm:
	testq	%rdi, %rdi
	jle	.L20
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, %rbx
	movq	%rsi, %r10
	movq	%rdx, %r11
	vmovapd	%xmm0, %xmm2
	movq	%rcx, %rsi
	movq	%r8, %r13
	movq	%r9, %rbp
	leaq	0(,%r10,8), %r14
	movq	%rcx, %r9
	addq	%r14, %rsi
	movl	$0, %r12d
	movl	$0, %edx
	movq	%rdx, %rcx
	jmp	.L3
	.p2align 5
.L7:
	vmulsd	(%rdi), %xmm2, %xmm0
	vmulsd	(%r15), %xmm0, %xmm0
	vaddsd	(%rax), %xmm0, %xmm0
	vmovsd	%xmm0, (%rax)
	addq	$8, %rax
	addq	$8, %r15
	cmpq	%rsi, %rax
	jne	.L7
.L9:
	addq	$1, %r8
	addq	$8, %rdi
	addq	%r10, %rdx
	cmpq	%r8, %r11
	je	.L5
.L6:
	leaq	0(%rbp,%rdx,8), %r15
	movq	%r9, %rax
	testq	%r10, %r10
	jg	.L7
	jmp	.L9
.L5:
	addq	$1, %rcx
	addq	%r14, %r9
	addq	%r14, %rsi
	addq	%r11, %r12
	cmpq	%rcx, %rbx
	je	.L1
.L3:
	movq	%r9, %rax
	testq	%r10, %r10
	jle	.L10
	.p2align 5
.L4:
	vmulsd	(%rax), %xmm1, %xmm0
	vmovsd	%xmm0, (%rax)
	addq	$8, %rax
	cmpq	%rsi, %rax
	jne	.L4
.L10:
	testq	%r11, %r11
	jle	.L5
	leaq	0(%r13,%r12,8), %rdi
	movl	$0, %edx
	movl	$0, %r8d
	jmp	.L6
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L20:
	ret
	.size	gemm, .-gemm
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
