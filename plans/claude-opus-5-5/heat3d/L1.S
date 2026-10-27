    .section .rodata
    .align 8
.Leighth:
    .double 0.125
    .text
    .globl heat3d
    .type heat3d, @function
heat3d:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    addq %rdi, %rdi
    movq %rsi, %r15
    imulq %rsi, %r15
    shlq $3, %r15
    leaq -2(%rsi), %r11
    andq $-4, %r11
    incq %r11
    vbroadcastsd .Leighth(%rip), %ymm15
.Lstep:
    testq %rdi, %rdi
    jle .Ldone
    movl $1, %r8d
.Li:
    leaq 1(%r8), %rax
    cmpq %rsi, %rax
    jge .Lswap
    movl $1, %r9d
.Lj:
    leaq 1(%r9), %rax
    cmpq %rsi, %rax
    jge .Lnexti
    movq %r8, %rax
    imulq %rsi, %rax
    addq %r9, %rax
    imulq %rsi, %rax
    leaq (%rcx,%rax,8), %r14
    leaq (%rdx,%rax,8), %rax
    movq %rax, %rbx
    subq %r15, %rbx
    leaq (%rax,%r15), %rbp
    leaq 0(,%rsi,8), %r12
    negq %r12
    addq %rax, %r12
    leaq (%rax,%rsi,8), %r13
    movl $1, %r10d
.Lvec:
    cmpq %r11, %r10
    jge .Ltail0
    vmovupd (%rax,%r10,8), %ymm1
    vaddpd %ymm1, %ymm1, %ymm5
    vmovupd (%rbp,%r10,8), %ymm2
    vsubpd %ymm5, %ymm2, %ymm2
    vaddpd (%rbx,%r10,8), %ymm2, %ymm2
    vmovupd (%r13,%r10,8), %ymm3
    vsubpd %ymm5, %ymm3, %ymm3
    vaddpd (%r12,%r10,8), %ymm3, %ymm3
    vmovupd 8(%rax,%r10,8), %ymm4
    vsubpd %ymm5, %ymm4, %ymm4
    vaddpd -8(%rax,%r10,8), %ymm4, %ymm4
    vmulpd %ymm15, %ymm2, %ymm2
    vaddpd %ymm2, %ymm1, %ymm0
    vmulpd %ymm15, %ymm3, %ymm3
    vaddpd %ymm3, %ymm0, %ymm0
    vmulpd %ymm15, %ymm4, %ymm4
    vaddpd %ymm4, %ymm0, %ymm0
    vmovupd %ymm0, (%r14,%r10,8)
    addq $4, %r10
    jmp .Lvec
.Ltail0:
    incq %r10
.Ltail:
    cmpq %rsi, %r10
    jge .Lnextj
    vmovsd -8(%rax,%r10,8), %xmm1
    vaddsd %xmm1, %xmm1, %xmm5
    vmovsd -8(%rbp,%r10,8), %xmm2
    vsubsd %xmm5, %xmm2, %xmm2
    vaddsd -8(%rbx,%r10,8), %xmm2, %xmm2
    vmovsd -8(%r13,%r10,8), %xmm3
    vsubsd %xmm5, %xmm3, %xmm3
    vaddsd -8(%r12,%r10,8), %xmm3, %xmm3
    vmovsd (%rax,%r10,8), %xmm4
    vsubsd %xmm5, %xmm4, %xmm4
    vaddsd -16(%rax,%r10,8), %xmm4, %xmm4
    vmulsd %xmm15, %xmm2, %xmm2
    vaddsd %xmm2, %xmm1, %xmm0
    vmulsd %xmm15, %xmm3, %xmm3
    vaddsd %xmm3, %xmm0, %xmm0
    vmulsd %xmm15, %xmm4, %xmm4
    vaddsd %xmm4, %xmm0, %xmm0
    vmovsd %xmm0, -8(%r14,%r10,8)
    incq %r10
    jmp .Ltail
.Lnextj:
    incq %r9
    jmp .Lj
.Lnexti:
    incq %r8
    jmp .Li
.Lswap:
    xchgq %rdx, %rcx
    decq %rdi
    jmp .Lstep
.Ldone:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    vzeroupper
    ret
    .size heat3d, .-heat3d
    .section .note.GNU-stack,"",@progbits
