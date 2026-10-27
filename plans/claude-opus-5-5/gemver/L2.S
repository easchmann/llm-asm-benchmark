    .text
    .globl gemver
    .type gemver, @function
gemver:
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq 48(%rsp), %r10
    movq 56(%rsp), %r11
    movq 64(%rsp), %rbx
    movq 72(%rsp), %r12
    movq %rdi, %r15
    andq $-4, %r15
    movq %rsi, %r13
    xorl %r14d, %r14d
.L1:
    cmpq %rdi, %r14
    jge .L2start
    vbroadcastsd (%rdx,%r14,8), %ymm2
    vbroadcastsd (%r8,%r14,8), %ymm3
    xorl %eax, %eax
.L1v:
    cmpq %r15, %rax
    jge .L1s
    vmulpd (%rcx,%rax,8), %ymm2, %ymm4
    vaddpd (%r13,%rax,8), %ymm4, %ymm4
    vmulpd (%r9,%rax,8), %ymm3, %ymm5
    vaddpd %ymm5, %ymm4, %ymm4
    vmovupd %ymm4, (%r13,%rax,8)
    addq $4, %rax
    jmp .L1v
.L1s:
    cmpq %rdi, %rax
    jge .L1next
    vmulsd (%rcx,%rax,8), %xmm2, %xmm4
    vaddsd (%r13,%rax,8), %xmm4, %xmm4
    vmulsd (%r9,%rax,8), %xmm3, %xmm5
    vaddsd %xmm5, %xmm4, %xmm4
    vmovsd %xmm4, (%r13,%rax,8)
    incq %rax
    jmp .L1s
.L1next:
    leaq (%r13,%rdi,8), %r13
    incq %r14
    jmp .L1
.L2start:
    movq %rsi, %r13
    xorl %r14d, %r14d
.L2:
    cmpq %rdi, %r14
    jge .L3start
    vmulsd (%rbx,%r14,8), %xmm1, %xmm2
    vbroadcastsd %xmm2, %ymm2
    xorl %eax, %eax
.L2v:
    cmpq %r15, %rax
    jge .L2s
    vmulpd (%r13,%rax,8), %ymm2, %ymm4
    vaddpd (%r11,%rax,8), %ymm4, %ymm4
    vmovupd %ymm4, (%r11,%rax,8)
    addq $4, %rax
    jmp .L2v
.L2s:
    cmpq %rdi, %rax
    jge .L2next
    vmulsd (%r13,%rax,8), %xmm2, %xmm4
    vaddsd (%r11,%rax,8), %xmm4, %xmm4
    vmovsd %xmm4, (%r11,%rax,8)
    incq %rax
    jmp .L2s
.L2next:
    leaq (%r13,%rdi,8), %r13
    incq %r14
    jmp .L2
.L3start:
    xorl %eax, %eax
.L3v:
    cmpq %r15, %rax
    jge .L3s
    vmovupd (%r11,%rax,8), %ymm4
    vaddpd (%r12,%rax,8), %ymm4, %ymm4
    vmovupd %ymm4, (%r11,%rax,8)
    addq $4, %rax
    jmp .L3v
.L3s:
    cmpq %rdi, %rax
    jge .L4start
    vmovsd (%r11,%rax,8), %xmm4
    vaddsd (%r12,%rax,8), %xmm4, %xmm4
    vmovsd %xmm4, (%r11,%rax,8)
    incq %rax
    jmp .L3s
.L4start:
    movq %rsi, %r13
    xorl %r14d, %r14d
.L4:
    cmpq %rdi, %r14
    jge .Ldone
    vxorpd %xmm2, %xmm2, %xmm2
    xorl %eax, %eax
.L4v:
    cmpq %r15, %rax
    jge .L4r
    vmovupd (%r13,%rax,8), %ymm4
    vmulpd (%r11,%rax,8), %ymm4, %ymm4
    vaddpd %ymm4, %ymm2, %ymm2
    addq $4, %rax
    jmp .L4v
.L4r:
    vextractf128 $1, %ymm2, %xmm4
    vaddpd %xmm4, %xmm2, %xmm2
    vunpckhpd %xmm2, %xmm2, %xmm4
    vaddsd %xmm4, %xmm2, %xmm2
.L4s:
    cmpq %rdi, %rax
    jge .L4store
    vmovsd (%r13,%rax,8), %xmm4
    vmulsd (%r11,%rax,8), %xmm4, %xmm4
    vaddsd %xmm4, %xmm2, %xmm2
    incq %rax
    jmp .L4s
.L4store:
    vmulsd %xmm0, %xmm2, %xmm2
    vaddsd (%r10,%r14,8), %xmm2, %xmm2
    vmovsd %xmm2, (%r10,%r14,8)
    leaq (%r13,%rdi,8), %r13
    incq %r14
    jmp .L4
.Ldone:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    vzeroupper
    ret
    .size gemver, .-gemver
    .section .note.GNU-stack,"",@progbits
