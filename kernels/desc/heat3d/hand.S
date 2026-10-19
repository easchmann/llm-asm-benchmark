	.file	"hand_heat3d.c"
	.text
	.globl	heat3d
	.type	heat3d, @function
heat3d:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	movq	%rdx, -64(%rsp)
	testq	%rdi, %rdi
	jle	.L1
	movq	%rsi, %rbp
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rbp, %rax
	leaq	1(%rbp), %rdx
	imulq	%rbp, %rdx
	movq	%rdx, %r8
	leaq	1(%rbp,%rbp), %rdx
	imulq	%rbp, %rdx
	movq	%rdx, %r9
	leaq	0(,%rbp,8), %r15
	leaq	2(%rbp), %rdx
	imulq	%rbp, %rdx
	salq	$3, %rdx
	subq	%r15, %rsi
	addq	%rdx, %rsi
	movq	%rsi, -40(%rsp)
	movq	%rcx, %rsi
	subq	%r15, %rsi
	addq	%rdx, %rsi
	movq	%rsi, -48(%rsp)
	movl	$0, %esi
	leaq	-1(%rbp), %r14
	vmovsd	.LC0(%rip), %xmm2
	vmovapd	%xmm2, %xmm1
	movq	%r8, -32(%rsp)
	movq	%r9, -24(%rsp)
	movq	%rax, %rdx
	movq	%rdi, -16(%rsp)
	movq	%rsi, %rdi
	jmp	.L3
.L12:
	addq	$1, -104(%rsp)
	movq	-96(%rsp), %rdi
	leaq	(%r10,%rdi,8), %rbx
	leaq	0(,%rax,8), %rdi
	leaq	(%r10,%rdi), %r11
	movq	-40(%rsp), %r9
	addq	%r9, %rdi
	leaq	(%r10,%rcx,8), %r9
	movq	-56(%rsp), %r8
	leaq	(%r8,%rsi,8), %r8
	movq	%rsi, %r13
	movl	$1, %r12d
	movq	%rsi, -88(%rsp)
	movq	%rdx, %rsi
	movq	%rax, -80(%rsp)
	movq	%rcx, -72(%rsp)
.L7:
	addq	$1, %r12
	movq	%r13, %rax
	addq	%rbp, %r13
	leaq	(%r10,%rax,8), %rdx
	movl	$1, %eax
.L4:
	movq	%rax, %rcx
	vmovsd	8(%rdx), %xmm4
	vaddsd	%xmm4, %xmm4, %xmm5
	addq	$1, %rax
	vmovsd	(%rbx,%rcx,8), %xmm0
	vsubsd	%xmm5, %xmm0, %xmm0
	vaddsd	(%r11,%rcx,8), %xmm0, %xmm0
	vmulsd	%xmm2, %xmm0, %xmm0
	vmovsd	(%rdi,%rcx,8), %xmm3
	vsubsd	%xmm5, %xmm3, %xmm3
	vaddsd	(%r9,%rcx,8), %xmm3, %xmm3
	vmulsd	%xmm2, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm0, %xmm0
	vmovsd	16(%rdx), %xmm3
	vsubsd	%xmm5, %xmm3, %xmm3
	vaddsd	(%rdx), %xmm3, %xmm3
	vmulsd	%xmm2, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm0, %xmm0
	vaddsd	%xmm4, %xmm0, %xmm0
	vmovsd	%xmm0, -8(%r8,%rax,8)
	addq	$8, %rdx
	cmpq	%r14, %rax
	jne	.L4
	addq	%r15, %rbx
	addq	%r15, %r11
	addq	%r15, %rdi
	addq	%r15, %r9
	addq	%r15, %r8
	cmpq	%r14, %r12
	jne	.L7
	movq	%rsi, %rdx
	movq	-88(%rsp), %rsi
	movq	-80(%rsp), %rax
	movq	-72(%rsp), %rcx
	addq	%rdx, %rsi
	addq	%rdx, -96(%rsp)
	addq	%rdx, %rax
	addq	%rdx, %rcx
	cmpq	%r14, -104(%rsp)
	jne	.L12
	movq	-8(%rsp), %rdi
	movq	-56(%rsp), %rcx
	movq	-24(%rsp), %rsi
	movq	%rsi, -88(%rsp)
	movq	-32(%rsp), %rsi
	movq	$1, -96(%rsp)
	movq	%rdi, -56(%rsp)
	movq	%rdx, %rdi
	movq	%rbp, %rax
.L6:
	addq	$1, -96(%rsp)
	movq	-88(%rsp), %rbx
	leaq	(%rcx,%rbx,8), %rbx
	leaq	0(,%rax,8), %r8
	leaq	(%rcx,%r8), %r11
	movq	-48(%rsp), %r10
	addq	%r10, %r8
	leaq	(%rcx,%rdi,8), %r10
	movq	-64(%rsp), %r9
	leaq	(%r9,%rsi,8), %r9
	movq	%rsi, %r13
	movl	$1, %r12d
	movq	%rsi, -80(%rsp)
	movq	%rdx, %rsi
	movq	%rax, -72(%rsp)
	movq	%rcx, -104(%rsp)
.L11:
	addq	$1, %r12
	movq	%r13, %rax
	addq	%rbp, %r13
	movq	-104(%rsp), %rdx
	leaq	(%rdx,%rax,8), %rdx
	movl	$1, %eax
.L8:
	movq	%rax, %rcx
	vmovsd	8(%rdx), %xmm4
	vaddsd	%xmm4, %xmm4, %xmm5
	addq	$1, %rax
	vmovsd	(%rbx,%rcx,8), %xmm0
	vsubsd	%xmm5, %xmm0, %xmm0
	vaddsd	(%r11,%rcx,8), %xmm0, %xmm0
	vmulsd	%xmm1, %xmm0, %xmm0
	vmovsd	(%r8,%rcx,8), %xmm3
	vsubsd	%xmm5, %xmm3, %xmm3
	vaddsd	(%r10,%rcx,8), %xmm3, %xmm3
	vmulsd	%xmm1, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm0, %xmm0
	vmovsd	16(%rdx), %xmm3
	vsubsd	%xmm5, %xmm3, %xmm3
	vaddsd	(%rdx), %xmm3, %xmm3
	vmulsd	%xmm1, %xmm3, %xmm3
	vaddsd	%xmm3, %xmm0, %xmm0
	vaddsd	%xmm4, %xmm0, %xmm0
	vmovsd	%xmm0, -8(%r9,%rax,8)
	addq	$8, %rdx
	cmpq	%r14, %rax
	jne	.L8
	addq	%r15, %rbx
	addq	%r15, %r11
	addq	%r15, %r8
	addq	%r15, %r10
	addq	%r15, %r9
	cmpq	%r14, %r12
	jne	.L11
	movq	%rsi, %rdx
	movq	-80(%rsp), %rsi
	movq	-72(%rsp), %rax
	movq	-104(%rsp), %rcx
	addq	%rdx, %rsi
	addq	%rdx, -88(%rsp)
	addq	%rdx, %rax
	addq	%rdx, %rdi
	cmpq	%r14, -96(%rsp)
	jne	.L6
	movq	-56(%rsp), %rdi
.L10:
	addq	$1, %rdi
	cmpq	%rdi, -16(%rsp)
	je	.L1
.L3:
	cmpq	$2, %rbp
	jle	.L10
	movq	-24(%rsp), %rsi
	movq	%rsi, -96(%rsp)
	movq	-32(%rsp), %rsi
	movq	$1, -104(%rsp)
	movq	%rdi, -8(%rsp)
	movq	-64(%rsp), %r10
	movq	%rcx, -56(%rsp)
	movq	%rdx, %rcx
	movq	%rbp, %rax
	jmp	.L12
.L1:
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.size	heat3d, .-heat3d
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	0
	.long	1069547520
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
