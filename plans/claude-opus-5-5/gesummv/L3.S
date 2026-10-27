    .text
    .globl gesummv
    .type gesummv, @function
gesummv:
    movq %rdi, %rax
    andq $-4, %rax
    xorl %r10d, %r10d
.Lrow:
    cmpq %rdi, %r10
    jge .Ldone
    vxorpd %xmm2, %xmm2, %xmm2
    vxorpd %xmm3, %xmm3, %xmm3
    xorl %r11d, %r11d
.Lvec:
    cmpq %rax, %r11
    jge .Lreduce
    vmovupd (%r8,%r11,8), %ymm4
    vmulpd (%rsi,%r11,8), %ymm4, %ymm5
    vaddpd %ymm5, %ymm2, %ymm2
    vmulpd (%rdx,%r11,8), %ymm4, %ymm5
    vaddpd %ymm5, %ymm3, %ymm3
    addq $4, %r11
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm2, %xmm4
    vaddpd %xmm4, %xmm2, %xmm2
    vunpckhpd %xmm2, %xmm2, %xmm4
    vaddsd %xmm4, %xmm2, %xmm2
    vextractf128 $1, %ymm3, %xmm4
    vaddpd %xmm4, %xmm3, %xmm3
    vunpckhpd %xmm3, %xmm3, %xmm4
    vaddsd %xmm4, %xmm3, %xmm3
.Ltail:
    cmpq %rdi, %r11
    jge .Lstore
    vmovsd (%r8,%r11,8), %xmm4
    vmulsd (%rsi,%r11,8), %xmm4, %xmm5
    vaddsd %xmm5, %xmm2, %xmm2
    vmulsd (%rdx,%r11,8), %xmm4, %xmm5
    vaddsd %xmm5, %xmm3, %xmm3
    incq %r11
    jmp .Ltail
.Lstore:
    vmovsd %xmm2, (%rcx,%r10,8)
    vmulsd %xmm0, %xmm2, %xmm2
    vmulsd %xmm1, %xmm3, %xmm3
    vaddsd %xmm3, %xmm2, %xmm2
    vmovsd %xmm2, (%r9,%r10,8)
    leaq (%rsi,%rdi,8), %rsi
    leaq (%rdx,%rdi,8), %rdx
    incq %r10
    jmp .Lrow
.Ldone:
    vzeroupper
    ret
    .size gesummv, .-gesummv
    .section .note.GNU-stack,"",@progbits
