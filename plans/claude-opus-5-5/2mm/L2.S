    .text
    .globl mm2
    .type mm2, @function
mm2:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq 56(%rsp), %r10
    movq 64(%rsp), %r11
    movq 72(%rsp), %r12
    movq %r8, %r13
    movq %r9, %r14
    movq %rsi, %r9
    andq $-4, %r9
    vxorpd %xmm5, %xmm5, %xmm5
    xorl %ebx, %ebx
.L1i:
    cmpq %rdi, %rbx
    jge .L2
    xorl %eax, %eax
.L1zv:
    cmpq %r9, %rax
    jge .L1zs
    vmovupd %ymm5, (%r13,%rax,8)
    addq $4, %rax
    jmp .L1zv
.L1zs:
    cmpq %rsi, %rax
    jge .L1k0
    vmovsd %xmm5, (%r13,%rax,8)
    incq %rax
    jmp .L1zs
.L1k0:
    movq %r10, %r15
    xorl %ebp, %ebp
.L1k:
    cmpq %rdx, %rbp
    jge .L1next
    vmulsd (%r14,%rbp,8), %xmm0, %xmm2
    vbroadcastsd %xmm2, %ymm2
    xorl %eax, %eax
.L1v:
    cmpq %r9, %rax
    jge .L1s
    vmovupd (%r15,%rax,8), %ymm4
    vmulpd %ymm2, %ymm4, %ymm4
    vaddpd (%r13,%rax,8), %ymm4, %ymm4
    vmovupd %ymm4, (%r13,%rax,8)
    addq $4, %rax
    jmp .L1v
.L1s:
    cmpq %rsi, %rax
    jge .L1nextk
    vmovsd (%r15,%rax,8), %xmm4
    vmulsd %xmm2, %xmm4, %xmm4
    vaddsd (%r13,%rax,8), %xmm4, %xmm4
    vmovsd %xmm4, (%r13,%rax,8)
    incq %rax
    jmp .L1s
.L1nextk:
    leaq (%r15,%rsi,8), %r15
    incq %rbp
    jmp .L1k
.L1next:
    leaq (%r13,%rsi,8), %r13
    leaq (%r14,%rdx,8), %r14
    incq %rbx
    jmp .L1i
.L2:
    movq %r8, %r13
    movq %r12, %r14
    movq %rcx, %r9
    andq $-4, %r9
    vbroadcastsd %xmm1, %ymm3
    xorl %ebx, %ebx
.L2i:
    cmpq %rdi, %rbx
    jge .Ldone
    xorl %eax, %eax
.L2bv:
    cmpq %r9, %rax
    jge .L2bs
    vmulpd (%r14,%rax,8), %ymm3, %ymm4
    vmovupd %ymm4, (%r14,%rax,8)
    addq $4, %rax
    jmp .L2bv
.L2bs:
    cmpq %rcx, %rax
    jge .L2j0
    vmulsd (%r14,%rax,8), %xmm1, %xmm4
    vmovsd %xmm4, (%r14,%rax,8)
    incq %rax
    jmp .L2bs
.L2j0:
    movq %r11, %r15
    xorl %ebp, %ebp
.L2j:
    cmpq %rsi, %rbp
    jge .L2next
    vbroadcastsd (%r13,%rbp,8), %ymm2
    xorl %eax, %eax
.L2v:
    cmpq %r9, %rax
    jge .L2s
    vmovupd (%r15,%rax,8), %ymm4
    vmulpd %ymm2, %ymm4, %ymm4
    vaddpd (%r14,%rax,8), %ymm4, %ymm4
    vmovupd %ymm4, (%r14,%rax,8)
    addq $4, %rax
    jmp .L2v
.L2s:
    cmpq %rcx, %rax
    jge .L2nextj
    vmovsd (%r15,%rax,8), %xmm4
    vmulsd %xmm2, %xmm4, %xmm4
    vaddsd (%r14,%rax,8), %xmm4, %xmm4
    vmovsd %xmm4, (%r14,%rax,8)
    incq %rax
    jmp .L2s
.L2nextj:
    leaq (%r15,%rcx,8), %r15
    incq %rbp
    jmp .L2j
.L2next:
    leaq (%r13,%rsi,8), %r13
    leaq (%r14,%rcx,8), %r14
    incq %rbx
    jmp .L2i
.Ldone:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    vzeroupper
    ret
    .size mm2, .-mm2
    .section .note.GNU-stack,"",@progbits
