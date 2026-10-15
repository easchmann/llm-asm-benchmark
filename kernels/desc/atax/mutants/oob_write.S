	.file	"atax.c"
	.text
	.p2align 4
	.globl	atax
	.type	atax, @function
atax:
	movq	%rcx, %r10
	movq	%rdx, %rax
	leaq	(%r8,%rsi,8), %rcx
	testq	%rsi, %rsi
	jle	.L6
	movq	%rcx, %r11
	movq	%r8, %rdx
	subq	%r8, %r11
	andl	$8, %r11d
	je	.L5
	leaq	8(%r8), %rdx
	movq	$0x000000000, (%r8)
	cmpq	%rcx, %rdx
	je	.L6
	.p2align 5
	.p2align 4
	.p2align 3
.L5:
	movq	$0x000000000, (%rdx)
	addq	$16, %rdx
	movq	$0x000000000, -8(%rdx)
	cmpq	%rcx, %rdx
	jne	.L5
.L6:
    movq	$0, (%rcx)		# mutant: write y[n], one past the end
	testq	%rdi, %rdi
	jle	.L32
	pushq	%rbx
	movq	%rax, %rcx
	leaq	0(,%rsi,8), %rbx
	leaq	(%r9,%rdi,8), %r11
.L13:
	movq	$0x000000000, (%r9)
	testq	%rsi, %rsi
	jg	.L11
	addq	$8, %r9
	addq	%rbx, %rcx
	cmpq	%r11, %r9
	jne	.L13
.L25:
	popq	%rbx
	ret
	.p2align 4,,10
	.p2align 3
.L33:
	vmovsd	(%r8), %xmm3
	vfmadd132sd	(%rcx), %xmm3, %xmm1
	vmovsd	%xmm1, (%r8)
	cmpq	$1, %rsi
	je	.L9
	movl	$1, %eax
	.p2align 6
	.p2align 4
	.p2align 3
.L10:
	vmovsd	(%rcx,%rax,8), %xmm0
	vmovsd	(%r8,%rax,8), %xmm2
	movq	%rax, %rdi
	vfmadd132sd	(%r9), %xmm2, %xmm0
	vmovsd	%xmm0, (%r8,%rax,8)
	addq	$1, %rax
	cmpq	%rdx, %rdi
	jne	.L10
.L9:
	addq	$8, %r9
	addq	%rbx, %rcx
	cmpq	%r11, %r9
	je	.L25
	movq	$0x000000000, (%r9)
.L11:
	vxorpd	%xmm1, %xmm1, %xmm1
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L8:
	vmovsd	(%rcx,%rax,8), %xmm0
	vmulsd	(%r10,%rax,8), %xmm0, %xmm0
	movq	%rax, %rdx
	leaq	1(%rax), %rax
	vaddsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r9)
	cmpq	%rax, %rsi
	jne	.L8
	jmp	.L33
.L32:
	ret
	.size	atax, .-atax
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
