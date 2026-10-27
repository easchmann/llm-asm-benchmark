    .text
    .globl spmv
    .type spmv, @function
spmv:
    movq 8(%rsp), %r10
.Lrow:
    testq %rdi, %rdi
    jle .Ldone
    movq (%rdx), %rax
    movq 8(%rdx), %rsi
    vxorpd %xmm0, %xmm0, %xmm0
.Lnz:
    cmpq %rsi, %rax
    jge .Lstore
    movq (%rcx,%rax,8), %r11
    vmovsd (%r8,%rax,8), %xmm1
    vmulsd (%r9,%r11,8), %xmm1, %xmm1
    vaddsd %xmm1, %xmm0, %xmm0
    incq %rax
    jmp .Lnz
.Lstore:
    vmovsd %xmm0, (%r10)
    addq $8, %rdx
    addq $8, %r10
    decq %rdi
    jmp .Lrow
.Ldone:
    ret
    .size spmv, .-spmv
    .section .note.GNU-stack,"",@progbits
