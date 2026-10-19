	.file	"hand_gesummv.c"
	.text
	.globl	gesummv
	.type	gesummv, @function
gesummv:
	testq	%rdi, %rdi
	jle	.L8
	pushq	%rbp
	pushq	%rbx
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rcx, %r10
	movq	%r9, %r11
	leaq	0(,%rdi,8), %rbx
	movq	%rax, %r9
	leaq	(%rcx,%rbx), %rbp
.L4:
	movq	%r10, %rcx
	nop	/* mutant: tmp[i] = 0 removed */
	movq	%r11, %rdx
	movq	$0x000000000, (%r11)
	movl	$0, %eax
	.p2align 6
.L3:
	vmovsd	(%r9,%rax,8), %xmm2
	vmulsd	(%r8,%rax,8), %xmm2, %xmm2
	vaddsd	(%rcx), %xmm2, %xmm2
	vmovsd	%xmm2, (%rcx)
	vmovsd	(%rsi,%rax,8), %xmm2
	vmulsd	(%r8,%rax,8), %xmm2, %xmm2
	vaddsd	(%rdx), %xmm2, %xmm2
	vmovsd	%xmm2, (%rdx)
	addq	$1, %rax
	cmpq	%rax, %rdi
	jne	.L3
	vmulsd	(%rcx), %xmm0, %xmm3
	vmulsd	%xmm1, %xmm2, %xmm2
	vaddsd	%xmm2, %xmm3, %xmm3
	vmovsd	%xmm3, (%rdx)
	addq	$8, %r10
	addq	$8, %r11
	addq	%rbx, %r9
	addq	%rbx, %rsi
	cmpq	%rbp, %r10
	jne	.L4
	popq	%rbx
	popq	%rbp
	ret
.L8:
	ret
	.size	gesummv, .-gesummv
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
