    .section .rodata
    .align 8
.Lthree:
    .double 3.0
    .text
    .globl jacobi1d
    .type jacobi1d, @function
jacobi1d:
    addq %rdi, %rdi
    leaq -2(%rsi), %r8
    andq $-4, %r8
    incq %r8
    leaq -1(%rsi), %r9
    vbroadcastsd .Lthree(%rip), %ymm2
.Lstep:
    testq %rdi, %rdi
    jle .Ldone
    movl $1, %eax
.Lvec:
    cmpq %r8, %rax
    jge .Ltail
    vmovupd -8(%rdx,%rax,8), %ymm0
    vaddpd (%rdx,%rax,8), %ymm0, %ymm0
    vaddpd 8(%rdx,%rax,8), %ymm0, %ymm0
    vdivpd %ymm2, %ymm0, %ymm0
    vmovupd %ymm0, (%rcx,%rax,8)
    addq $4, %rax
    jmp .Lvec
.Ltail:
    cmpq %r9, %rax
    jge .Lswap
    vmovsd -8(%rdx,%rax,8), %xmm0
    vaddsd (%rdx,%rax,8), %xmm0, %xmm0
    vaddsd 8(%rdx,%rax,8), %xmm0, %xmm0
    vdivsd %xmm2, %xmm0, %xmm0
    vmovsd %xmm0, (%rcx,%rax,8)
    incq %rax
    jmp .Ltail
.Lswap:
    xchgq %rdx, %rcx
    decq %rdi
    jmp .Lstep
.Ldone:
    vzeroupper
    ret
    .size jacobi1d, .-jacobi1d
    .section .note.GNU-stack,"",@progbits
