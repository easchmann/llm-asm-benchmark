    .text
    .globl bicg
    .type bicg, @function
bicg:
    movq 8(%rsp), %r10
    movq %rdi, %r11
    andq $-4, %r11
    vxorpd %xmm0, %xmm0, %xmm0
    xorl %eax, %eax
.Lclearv:
    cmpq %r11, %rax
    jge .Lclears
    vmovupd %ymm0, (%rcx,%rax,8)
    addq $4, %rax
    jmp .Lclearv
.Lclears:
    cmpq %rdi, %rax
    jge .Lrow
    vmovsd %xmm0, (%rcx,%rax,8)
    incq %rax
    jmp .Lclears
.Lrow:
    testq %rsi, %rsi
    jle .Ldone
    vxorpd %xmm2, %xmm2, %xmm2
    vbroadcastsd (%r10), %ymm3
    xorl %eax, %eax
.Lvec:
    cmpq %r11, %rax
    jge .Lreduce
    vmovupd (%rdx,%rax,8), %ymm4
    vmulpd (%r9,%rax,8), %ymm4, %ymm5
    vaddpd %ymm5, %ymm2, %ymm2
    vmulpd %ymm4, %ymm3, %ymm5
    vaddpd (%rcx,%rax,8), %ymm5, %ymm5
    vmovupd %ymm5, (%rcx,%rax,8)
    addq $4, %rax
    jmp .Lvec
.Lreduce:
    vextractf128 $1, %ymm2, %xmm4
    vaddpd %xmm4, %xmm2, %xmm2
    vunpckhpd %xmm2, %xmm2, %xmm4
    vaddsd %xmm4, %xmm2, %xmm2
.Ltail:
    cmpq %rdi, %rax
    jge .Lstore
    vmovsd (%rdx,%rax,8), %xmm4
    vmulsd (%r9,%rax,8), %xmm4, %xmm5
    vaddsd %xmm5, %xmm2, %xmm2
    vmulsd %xmm4, %xmm3, %xmm5
    vaddsd (%rcx,%rax,8), %xmm5, %xmm5
    vmovsd %xmm5, (%rcx,%rax,8)
    incq %rax
    jmp .Ltail
.Lstore:
    vmovsd %xmm2, (%r8)
    leaq (%rdx,%rdi,8), %rdx
    addq $8, %r10
    addq $8, %r8
    decq %rsi
    jmp .Lrow
.Ldone:
    vzeroupper
    ret
    .size bicg, .-bicg
    .section .note.GNU-stack,"",@progbits
