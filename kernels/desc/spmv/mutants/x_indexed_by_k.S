	.file	"hand_spmv.c"
	.text
	.globl	spmv
	.type	spmv, @function
spmv:
	pushq	%rbp
	pushq	%rbx
	movq	24(%rsp), %rbp
	testq	%rdi, %rdi
	jle	.L1
	movq	%rdi, %r10
	movq	%rdx, %r11
	movq	%rcx, %rsi
	movl	$0, %edi
.L5:
	movq	%rdi, %rbx
	movq	(%r11,%rdi,8), %rax
	movq	8(%r11,%rdi,8), %rcx
	cmpq	%rcx, %rax
	jge	.L6
	vxorpd	%xmm1, %xmm1, %xmm1
	.p2align 5
.L4:
	movq	(%rsi,%rax,8), %rdx
	vmovsd	(%r9,%rax,8), %xmm0	/* mutant: x indexed by k instead of col[k] */
	vmulsd	(%r8,%rax,8), %xmm0, %xmm0
	vaddsd	%xmm0, %xmm1, %xmm1
	addq	$1, %rax
	cmpq	%rcx, %rax
	jne	.L4
.L3:
	vmovsd	%xmm1, 0(%rbp,%rbx,8)
	addq	$1, %rdi
	cmpq	%rdi, %r10
	jne	.L5
.L1:
	popq	%rbx
	popq	%rbp
	ret
.L6:
	vxorpd	%xmm1, %xmm1, %xmm1
	jmp	.L3
	.size	spmv, .-spmv
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
