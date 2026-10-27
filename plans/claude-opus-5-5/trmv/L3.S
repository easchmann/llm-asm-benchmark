    .text
    .globl trmv
    .type trmv, @function
trmv:
    xorl %r8d, %r8d
.Lrow:
    cmpq %rdi, %r8
    jge .Ldone
    leaq 1(%r8), %r10
    movq %r10, %r11
    andq $-4, %r11
    vxorpd %xmm0, %xmm0, %xmm0
    xorl %r9d, %r9d
.Lvec:
    cmpq %r11, %r9
    jge .Lreduce
    vmovupd (%rsi,%r9,8), %ymm1
    vmulpd (%rdx,%r9,8), %ymm1, %ymm1
    vaddpd %ymm1, %ymm0, %ymm0
    addq $4, %r9
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm0, %xmm1
    vaddpd %xmm1, %xmm0, %xmm0
    vunpckhpd %xmm0, %xmm0, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
.Ltail:
    cmpq %r10, %r9
    jge .Lstore
    vmovsd (%rsi,%r9,8), %xmm1
    vmulsd (%rdx,%r9,8), %xmm1, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
    incq %r9
    jmp .Ltail
.Lstore:
    vmovsd %xmm0, (%rcx,%r8,8)
    leaq (%rsi,%rdi,8), %rsi
    incq %r8
    jmp .Lrow
.Ldone:
    vzeroupper
    ret
    .size trmv, .-trmv
    .section .note.GNU-stack,"",@progbits
