	.file	"hand_2mm.c"
	.text
	.globl	mm2
	.type	mm2, @function
mm2:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, -16(%rsp)
	testq	%rdi, %rdi
	jle	.L1
	movq	%rsi, %r11
	movq	%rdx, %rbp
	movq	%rcx, %rbx
	vmovapd	%xmm0, %xmm2
	leaq	0(,%rdx,8), %r13
	leaq	(%r9,%r13), %rdi
	leaq	0(,%rsi,8), %rdx
	movl	$0, %esi
	movl	$0, %ecx
	movl	$0, %r14d
	movq	%r14, -32(%rsp)
	movq	%rbx, -8(%rsp)
	jmp	.L3
.L8:
	leaq	(%r8,%rcx,8), %r10
	movq	56(%rsp), %r12
	leaq	(%r9,%rsi,8), %r15
	movl	$0, %ebx
	movq	%rcx, -24(%rsp)
.L6:
	movq	%r10, %r14
	movq	$0x000000000, (%r10)
	testq	%rbp, %rbp
	jle	.L4
	movq	%r12, %rcx
	movq	%r15, %rax
	.p2align 5
.L5:
	vmulsd	(%rax), %xmm2, %xmm0
	vmulsd	(%rcx), %xmm0, %xmm0
	vaddsd	(%r14), %xmm0, %xmm0
	vmovsd	%xmm0, (%r14)
	addq	$8, %rax
	addq	%rdx, %rcx
	cmpq	%rdi, %rax
	jne	.L5
.L4:
	addq	$1, %rbx
	addq	$8, %r10
	addq	$8, %r12
	cmpq	%rbx, %r11
	jne	.L6
	movq	-24(%rsp), %rcx
.L9:
	movq	-32(%rsp), %rax
	addq	$1, %rax
	addq	%r11, %rcx
	addq	%rbp, %rsi
	addq	%r13, %rdi
	cmpq	%rax, -16(%rsp)
	je	.L7
	movq	%rax, -32(%rsp)
.L3:
	testq	%r11, %r11
	jg	.L8
	jmp	.L9
.L7:
	movq	-32(%rsp), %r14
	movq	-8(%rsp), %rbx
	leaq	(%r8,%rdx), %rsi
	leaq	0(,%rbx,8), %rdi
	movl	$0, %r13d
	movl	$0, %r12d
	movl	$0, %ecx
	movq	%r14, -24(%rsp)
	jmp	.L10
.L14:
	movq	72(%rsp), %rax
	leaq	(%rax,%r12,8), %r10
	movq	56(%rsp), %r14	/* mutant: C read from B stack slot (56 instead of 64) */
	leaq	(%r8,%r13,8), %r15
	movl	$0, %ebp
	movq	%rcx, -32(%rsp)
.L13:
	movq	%r10, %r9
	vmulsd	(%r10), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	testq	%r11, %r11
	jle	.L11
	movq	%r14, %rcx
	movq	%r15, %rax
	.p2align 5
.L12:
	vmovsd	(%rax), %xmm0
	vmulsd	(%rcx), %xmm0, %xmm0
	vaddsd	(%r9), %xmm0, %xmm0
	vmovsd	%xmm0, (%r9)
	addq	$8, %rax
	addq	%rdi, %rcx
	cmpq	%rsi, %rax
	jne	.L12
.L11:
	addq	$1, %rbp
	addq	$8, %r10
	addq	$8, %r14
	cmpq	%rbp, %rbx
	jne	.L13
	movq	-32(%rsp), %rcx
.L15:
	leaq	1(%rcx), %rax
	addq	%rbx, %r12
	addq	%r11, %r13
	addq	%rdx, %rsi
	cmpq	%rcx, -24(%rsp)
	je	.L1
	movq	%rax, %rcx
.L10:
	testq	%rbx, %rbx
	jg	.L14
	jmp	.L15
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.size	mm2, .-mm2
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
