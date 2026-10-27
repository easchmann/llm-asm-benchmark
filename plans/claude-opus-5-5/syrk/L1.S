    .text
    .globl syrk
    .type syrk, @function
syrk:
    pushq %rbx
    movq %rsi, %rbx
    andq $-4, %rbx
    movq %rdx, %r9
    movq %rcx, %r8
    xorl %edx, %edx
.Li:
    cmpq %rdi, %rdx
    jge .Ldone
    movq %rcx, %r10
    xorl %r11d, %r11d
.Lj:
    cmpq %rdx, %r11
    jg .Lnexti
    vxorpd %xmm2, %xmm2, %xmm2
    xorl %eax, %eax
.Lvec:
    cmpq %rbx, %rax
    jge .Lreduce
    vmovupd (%r8,%rax,8), %ymm3
    vmulpd (%r10,%rax,8), %ymm3, %ymm3
    vaddpd %ymm3, %ymm2, %ymm2
    addq $4, %rax
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm2, %xmm3
    vaddpd %xmm3, %xmm2, %xmm2
    vunpckhpd %xmm2, %xmm2, %xmm3
    vaddsd %xmm3, %xmm2, %xmm2
.Ltail:
    cmpq %rsi, %rax
    jge .Lstore
    vmovsd (%r8,%rax,8), %xmm3
    vmulsd (%r10,%rax,8), %xmm3, %xmm3
    vaddsd %xmm3, %xmm2, %xmm2
    incq %rax
    jmp .Ltail
.Lstore:
    vmulsd (%r9,%r11,8), %xmm1, %xmm3
    vmulsd %xmm0, %xmm2, %xmm2
    vaddsd %xmm2, %xmm3, %xmm3
    vmovsd %xmm3, (%r9,%r11,8)
    leaq (%r10,%rsi,8), %r10
    incq %r11
    jmp .Lj
.Lnexti:
    leaq (%r9,%rdi,8), %r9
    leaq (%r8,%rsi,8), %r8
    incq %rdx
    jmp .Li
.Ldone:
    popq %rbx
    vzeroupper
    ret
    .size syrk, .-syrk
    .section .note.GNU-stack,"",@progbits
