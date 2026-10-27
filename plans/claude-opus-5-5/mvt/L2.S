    .text
    .globl mvt
    .type mvt, @function
mvt:
    movq %rdi, %rax
    andq $-4, %rax
    xorl %r10d, %r10d
.Lrow:
    cmpq %rdi, %r10
    jge .Ldone
    vxorpd %xmm2, %xmm2, %xmm2
    vbroadcastsd (%r8,%r10,8), %ymm3
    xorl %r11d, %r11d
.Lvec:
    cmpq %rax, %r11
    jge .Lreduce
    vmovupd (%r9,%r11,8), %ymm4
    vmulpd (%rcx,%r11,8), %ymm4, %ymm5
    vaddpd %ymm5, %ymm2, %ymm2
    vmulpd %ymm3, %ymm4, %ymm5
    vaddpd (%rdx,%r11,8), %ymm5, %ymm5
    vmovupd %ymm5, (%rdx,%r11,8)
    addq $4, %r11
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm2, %xmm4
    vaddpd %xmm4, %xmm2, %xmm2
    vunpckhpd %xmm2, %xmm2, %xmm4
    vaddsd %xmm4, %xmm2, %xmm2
.Ltail:
    cmpq %rdi, %r11
    jge .Lstore
    vmovsd (%r9,%r11,8), %xmm4
    vmulsd (%rcx,%r11,8), %xmm4, %xmm5
    vaddsd %xmm5, %xmm2, %xmm2
    vmulsd %xmm3, %xmm4, %xmm5
    vaddsd (%rdx,%r11,8), %xmm5, %xmm5
    vmovsd %xmm5, (%rdx,%r11,8)
    incq %r11
    jmp .Ltail
.Lstore:
    vaddsd (%rsi,%r10,8), %xmm2, %xmm2
    vmovsd %xmm2, (%rsi,%r10,8)
    leaq (%r9,%rdi,8), %r9
    incq %r10
    jmp .Lrow
.Ldone:
    vzeroupper
    ret
    .size mvt, .-mvt
    .section .note.GNU-stack,"",@progbits
