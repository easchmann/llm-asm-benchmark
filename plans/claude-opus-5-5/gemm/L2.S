    .text
    .globl gemm
    .type gemm, @function
gemm:
    pushq %rbx
    movq %rsi, %rbx
    andq $-4, %rbx
    vbroadcastsd %xmm1, %ymm5
.Lrow:
    testq %rdi, %rdi
    jle .Ldone
    xorl %eax, %eax
.Lbetav:
    cmpq %rbx, %rax
    jge .Lbetas
    vmulpd (%rcx,%rax,8), %ymm5, %ymm3
    vmovupd %ymm3, (%rcx,%rax,8)
    addq $4, %rax
    jmp .Lbetav
.Lbetas:
    cmpq %rsi, %rax
    jge .Lk0
    vmulsd (%rcx,%rax,8), %xmm1, %xmm3
    vmovsd %xmm3, (%rcx,%rax,8)
    incq %rax
    jmp .Lbetas
.Lk0:
    movq %r9, %r10
    xorl %r11d, %r11d
.Lk:
    cmpq %rdx, %r11
    jge .Lnext
    vmulsd (%r8,%r11,8), %xmm0, %xmm2
    vbroadcastsd %xmm2, %ymm2
    xorl %eax, %eax
.Lvec:
    cmpq %rbx, %rax
    jge .Ltail
    vmovupd (%r10,%rax,8), %ymm3
    vmulpd %ymm2, %ymm3, %ymm3
    vaddpd (%rcx,%rax,8), %ymm3, %ymm3
    vmovupd %ymm3, (%rcx,%rax,8)
    addq $4, %rax
    jmp .Lvec
.Ltail:
    cmpq %rsi, %rax
    jge .Lnextk
    vmovsd (%r10,%rax,8), %xmm3
    vmulsd %xmm2, %xmm3, %xmm3
    vaddsd (%rcx,%rax,8), %xmm3, %xmm3
    vmovsd %xmm3, (%rcx,%rax,8)
    incq %rax
    jmp .Ltail
.Lnextk:
    leaq (%r10,%rsi,8), %r10
    incq %r11
    jmp .Lk
.Lnext:
    leaq (%rcx,%rsi,8), %rcx
    leaq (%r8,%rdx,8), %r8
    decq %rdi
    jmp .Lrow
.Ldone:
    popq %rbx
    vzeroupper
    ret
    .size gemm, .-gemm
    .section .note.GNU-stack,"",@progbits
