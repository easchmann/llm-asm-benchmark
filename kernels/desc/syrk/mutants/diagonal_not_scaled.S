	.file	"hand_syrk.c"
	.text
	.globl	syrk
	.type	syrk, @function
syrk:
	testq	%rdi, %rdi
	jle	.L12
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdi, %rbp
	movq	%rsi, %r10
	vmovapd	%xmm0, %xmm2
	movq	%rcx, %r13
	leaq	0(,%rdi,8), %r14
	movq	%rsi, %r12
	negq	%r12
	salq	$3, %r12
	leaq	0(,%rsi,8), %r8
	movl	$0, %r11d
	movl	$0, %ebx
	movl	$0, %edi
	movq	%rdx, %rcx
.L3:
	movl	$0, %eax
	movq	%rcx, %rsi
	.p2align 5
.L4:
	vmulsd	(%rcx,%rax,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%rax,8)
	addq	$1, %rax
	cmpq	%rdi, %rax
	jl	.L4	/* mutant: diagonal not scaled by beta (j < i) */
	testq	%r10, %r10
	jle	.L5
	leaq	0(%r13,%rbx,8), %r9
	movl	$0, %edx
.L6:
	leaq	(%r9,%r11), %r15
	movl	$0, %eax
	.p2align 5
.L7:
	vmulsd	(%r9), %xmm2, %xmm0
	vmulsd	(%r15), %xmm0, %xmm0
	vaddsd	(%rsi,%rax,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rax,8)
	addq	$1, %rax
	addq	%r8, %r15
	cmpq	%rdi, %rax
	jle	.L7
	addq	$1, %rdx
	addq	$8, %r9
	cmpq	%rdx, %r10
	jne	.L6
.L5:
	addq	$1, %rdi
	addq	%r14, %rcx
	addq	%r10, %rbx
	addq	%r12, %r11
	cmpq	%rdi, %rbp
	jne	.L3
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L12:
	ret
	.size	syrk, .-syrk
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
