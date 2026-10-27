    .text
    .globl trmm
    .type trmm, @function
trmm:
    pushq %rbx
    pushq %r12
    movq %rsi, %r12
    andq $-4, %r12
    vbroadcastsd %xmm0, %ymm0
    xorl %r9d, %r9d
    movq %rcx, %r8
.Li:
    cmpq %rdi, %r9
    jge .Ldone
    leaq 1(%r9), %r11
    leaq (%r8,%rsi,8), %r10
    movq %r11, %rax
    imulq %rdi, %rax
    addq %r9, %rax
    leaq (%rdx,%rax,8), %rax
.Lk:
    cmpq %rdi, %r11
    jge .Lscale
    vbroadcastsd (%rax), %ymm2
    xorl %ebx, %ebx
.Lvec:
    cmpq %r12, %rbx
    jge .Ltail
    vmovupd (%r10,%rbx,8), %ymm3
    vmulpd %ymm2, %ymm3, %ymm3
    vaddpd (%r8,%rbx,8), %ymm3, %ymm3
    vmovupd %ymm3, (%r8,%rbx,8)
    addq $4, %rbx
    jmp .Lvec
.Ltail:
    cmpq %rsi, %rbx
    jge .Lnextk
    vmovsd (%r10,%rbx,8), %xmm3
    vmulsd %xmm2, %xmm3, %xmm3
    vaddsd (%r8,%rbx,8), %xmm3, %xmm3
    vmovsd %xmm3, (%r8,%rbx,8)
    incq %rbx
    jmp .Ltail
.Lnextk:
    leaq (%r10,%rsi,8), %r10
    leaq (%rax,%rdi,8), %rax
    incq %r11
    jmp .Lk
.Lscale:
    xorl %ebx, %ebx
.Lscalev:
    cmpq %r12, %rbx
    jge .Lscales
    vmulpd (%r8,%rbx,8), %ymm0, %ymm3
    vmovupd %ymm3, (%r8,%rbx,8)
    addq $4, %rbx
    jmp .Lscalev
.Lscales:
    cmpq %rsi, %rbx
    jge .Lnexti
    vmulsd (%r8,%rbx,8), %xmm0, %xmm3
    vmovsd %xmm3, (%r8,%rbx,8)
    incq %rbx
    jmp .Lscales
.Lnexti:
    leaq (%r8,%rsi,8), %r8
    incq %r9
    jmp .Li
.Ldone:
    popq %r12
    popq %rbx
    vzeroupper
    ret
    .size trmm, .-trmm
    .section .note.GNU-stack,"",@progbits
