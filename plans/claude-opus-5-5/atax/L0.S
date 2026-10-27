    .text
    .globl atax
    .type atax, @function
atax:
    movq %rsi, %r11
    andq $-4, %r11
    vxorpd %xmm0, %xmm0, %xmm0
    xorl %r10d, %r10d
.Lclearv:
    cmpq %r11, %r10
    jge .Lclears
    vmovupd %ymm0, (%r8,%r10,8)
    addq $4, %r10
    jmp .Lclearv
.Lclears:
    cmpq %rsi, %r10
    jge .Lrow
    vmovsd %xmm0, (%r8,%r10,8)
    incq %r10
    jmp .Lclears
.Lrow:
    testq %rdi, %rdi
    jle .Ldone
    vxorpd %xmm0, %xmm0, %xmm0
    xorl %r10d, %r10d
.Ldotv:
    cmpq %r11, %r10
    jge .Ldotr
    vmovupd (%rdx,%r10,8), %ymm1
    vmulpd (%rcx,%r10,8), %ymm1, %ymm1
    vaddpd %ymm1, %ymm0, %ymm0
    addq $4, %r10
    jmp .Ldotv
.Ldotr:
    vextractf128 $1, %ymm0, %xmm1
    vaddpd %xmm1, %xmm0, %xmm0
    vunpckhpd %xmm0, %xmm0, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
.Ldots:
    cmpq %rsi, %r10
    jge .Ldotdone
    vmovsd (%rdx,%r10,8), %xmm1
    vmulsd (%rcx,%r10,8), %xmm1, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
    incq %r10
    jmp .Ldots
.Ldotdone:
    vmovsd %xmm0, (%r9)
    vbroadcastsd (%r9), %ymm2
    xorl %r10d, %r10d
.Lupdv:
    cmpq %r11, %r10
    jge .Lupds
    vmovupd (%rdx,%r10,8), %ymm3
    vmulpd %ymm2, %ymm3, %ymm3
    vaddpd (%r8,%r10,8), %ymm3, %ymm3
    vmovupd %ymm3, (%r8,%r10,8)
    addq $4, %r10
    jmp .Lupdv
.Lupds:
    cmpq %rsi, %r10
    jge .Lnext
    vmovsd (%rdx,%r10,8), %xmm3
    vmulsd %xmm2, %xmm3, %xmm3
    vaddsd (%r8,%r10,8), %xmm3, %xmm3
    vmovsd %xmm3, (%r8,%r10,8)
    incq %r10
    jmp .Lupds
.Lnext:
    leaq (%rdx,%rsi,8), %rdx
    addq $8, %r9
    decq %rdi
    jmp .Lrow
.Ldone:
    vzeroupper
    ret
    .size atax, .-atax
    .section .note.GNU-stack,"",@progbits
