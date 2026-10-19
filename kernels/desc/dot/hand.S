	.file	"hand_dot.c"
	.text
	.globl	dot
	.type	dot, @function
dot:
	testq	%rdi, %rdi
	jle	.L4
	movl	$0, %eax
	vxorpd	%xmm1, %xmm1, %xmm1
	.p2align 5
.L3:
	vmovsd	(%rsi,%rax,8), %xmm0
	vmulsd	(%rdx,%rax,8), %xmm0, %xmm0
	vaddsd	%xmm0, %xmm1, %xmm1
	addq	$1, %rax
	cmpq	%rax, %rdi
	jne	.L3
.L1:
	vmovapd	%xmm1, %xmm0
	ret
.L4:
	vxorpd	%xmm1, %xmm1, %xmm1
	jmp	.L1
	.size	dot, .-dot
	.ident	"GCC: (GNU) 14.3.1 20250617 (Red Hat 14.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
