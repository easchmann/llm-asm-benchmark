	.file	"bicg.c"
	.text
	.p2align 4
	.globl	bicg
	.type	bicg, @function
bicg:
	movq	%rsi, %r11
	leaq	(%rcx,%rdi,8), %rsi
	testq	%rdi, %rdi
	jle	.L6
	movq	%rsi, %r10
	movq	%rcx, %rax
	subq	%rcx, %r10
	andl	$8, %r10d
	je	.L5
	leaq	8(%rcx), %rax
	movq	$0x000000000, (%rcx)
	cmpq	%rsi, %rax
	je	.L6
	.p2align 5
	.p2align 4
	.p2align 3
.L5:
	movq	$0x000000000, (%rax)
	addq	$16, %rax
	movq	$0x000000000, -8(%rax)
	cmpq	%rsi, %rax
	jne	.L5
.L6:
	testq	%r11, %r11
	jle	.L31
	pushq	%rbx
	movq	%r8, %r10
	movq	16(%rsp), %rsi
	leaq	(%r8,%r11,8), %r11
	movq	$0x000000000, (%r8)
	leaq	0(,%rdi,8), %rbx
	testq	%rdi, %rdi
	jle	.L32
	.p2align 4
	.p2align 3
.L7:
	xorl	%eax, %eax
	.p2align 6
	.p2align 4
	.p2align 3
.L9:
	vmovsd	(%rcx,%rax,8), %xmm1
	vmovsd	(%rsi), %xmm0
	vfmadd132sd	(%rdx,%rax,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%rax,8)
	vmovsd	(%rdx,%rax,8), %xmm0
	vmovsd	(%r8), %xmm2
	vfmadd132sd	(%r9,%rax,8), %xmm2, %xmm0
	addq	$1, %rax
	vmovsd	%xmm0, (%r8)
	cmpq	%rax, %rdi
	jne	.L9
	addq	$8, %r10
	addq	$8, %rsi
	addq	%rbx, %rdx
	cmpq	%r11, %r10
	je	.L26
	movq	$0x000000000, (%r10)
	movq	%r10, %r8
	jmp	.L7
.L33:
	movq	$0x000000000, 8(%r10)
	addq	$16, %r10
	movq	$0x000000000, 8(%rax)
.L8:
	leaq	8(%r10), %rax
	cmpq	%r11, %rax
	jne	.L33
.L26:
	popq	%rbx
	ret
.L32:
	movq	%r11, %rax
	subq	%r8, %rax
	testb	$8, %al
	jne	.L8
	addq	$8, %r10
	movq	$0x000000000, (%r10)
	jmp	.L8
.L31:
	ret
	.size	bicg, .-bicg
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
