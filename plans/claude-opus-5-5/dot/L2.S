    .text
    .globl dot
    .type dot, @function
dot:
    vxorpd %xmm0, %xmm0, %xmm0
    movq %rdi, %rax
    andq $-4, %rax
    xorl %ecx, %ecx
.Lvec:
    cmpq %rax, %rcx
    jge .Lreduce
    vmovupd (%rsi,%rcx,8), %ymm1
    vmulpd (%rdx,%rcx,8), %ymm1, %ymm1
    vaddpd %ymm1, %ymm0, %ymm0
    addq $4, %rcx
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm0, %xmm1
    vaddpd %xmm1, %xmm0, %xmm0
    vunpckhpd %xmm0, %xmm0, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
.Ltail:
    cmpq %rdi, %rcx
    jge .Ldone
    vmovsd (%rsi,%rcx,8), %xmm1
    vmulsd (%rdx,%rcx,8), %xmm1, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
    incq %rcx
    jmp .Ltail
.Ldone:
    vzeroupper
    ret
    .size dot, .-dot
    .section .note.GNU-stack,"",@progbits
