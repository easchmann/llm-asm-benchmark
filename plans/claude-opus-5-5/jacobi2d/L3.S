    .section .rodata
    .align 8
.Lfifth:
    .double 0.2
    .text
    .globl jacobi2d
    .type jacobi2d, @function
jacobi2d:
    pushq %rbx
    pushq %r12
    addq %rdi, %rdi
    leaq -2(%rsi), %rax
    andq $-4, %rax
    incq %rax
    vbroadcastsd .Lfifth(%rip), %ymm15
.Lstep:
    testq %rdi, %rdi
    jle .Ldone
    movq %rdx, %rbx
    leaq (%rdx,%rsi,8), %r10
    leaq (%r10,%rsi,8), %r12
    leaq (%rcx,%rsi,8), %r11
    movl $1, %r8d
.Lrow:
    leaq 1(%r8), %r9
    cmpq %rsi, %r9
    jge .Lswap
    movl $1, %r9d
.Lvec:
    cmpq %rax, %r9
    jge .Ltail0
    vmovupd (%r10,%r9,8), %ymm0
    vaddpd -8(%r10,%r9,8), %ymm0, %ymm0
    vaddpd 8(%r10,%r9,8), %ymm0, %ymm0
    vaddpd (%rbx,%r9,8), %ymm0, %ymm0
    vaddpd (%r12,%r9,8), %ymm0, %ymm0
    vmulpd %ymm15, %ymm0, %ymm0
    vmovupd %ymm0, (%r11,%r9,8)
    addq $4, %r9
    jmp .Lvec
.Ltail0:
    incq %r9
.Ltail:
    cmpq %rsi, %r9
    jge .Lnext
    vmovsd -8(%r10,%r9,8), %xmm0
    vaddsd -16(%r10,%r9,8), %xmm0, %xmm0
    vaddsd (%r10,%r9,8), %xmm0, %xmm0
    vaddsd -8(%rbx,%r9,8), %xmm0, %xmm0
    vaddsd -8(%r12,%r9,8), %xmm0, %xmm0
    vmulsd %xmm15, %xmm0, %xmm0
    vmovsd %xmm0, -8(%r11,%r9,8)
    incq %r9
    jmp .Ltail
.Lnext:
    movq %r10, %rbx
    movq %r12, %r10
    leaq (%r12,%rsi,8), %r12
    leaq (%r11,%rsi,8), %r11
    incq %r8
    jmp .Lrow
.Lswap:
    xchgq %rdx, %rcx
    decq %rdi
    jmp .Lstep
.Ldone:
    popq %r12
    popq %rbx
    vzeroupper
    ret
    .size jacobi2d, .-jacobi2d
    .section .note.GNU-stack,"",@progbits
