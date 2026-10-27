    .text
    .globl axpy
    .type axpy, @function
axpy:
    vbroadcastsd %xmm0, %ymm0
    movq %rdi, %rax
    andq $-4, %rax
    xorl %ecx, %ecx
.Lvec:
    cmpq %rax, %rcx
    jge .Ltail
    vmovupd (%rsi,%rcx,8), %ymm1
    vmulpd %ymm0, %ymm1, %ymm1
    vaddpd (%rdx,%rcx,8), %ymm1, %ymm1
    vmovupd %ymm1, (%rdx,%rcx,8)
    addq $4, %rcx
    jmp .Lvec
.Ltail:
    cmpq %rdi, %rcx
    jge .Ldone
    vmovsd (%rsi,%rcx,8), %xmm1
    vmulsd %xmm0, %xmm1, %xmm1
    vaddsd (%rdx,%rcx,8), %xmm1, %xmm1
    vmovsd %xmm1, (%rdx,%rcx,8)
    incq %rcx
    jmp .Ltail
.Ldone:
    vzeroupper
    ret
    .size axpy, .-axpy
    .section .note.GNU-stack,"",@progbits
