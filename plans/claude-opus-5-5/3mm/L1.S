    .macro PRODUCT x, y, z, p, r, q
    movq \x, %r13
    movq \y, %r14
    movq \z, %r12
    movq \q, %r10
    andq $-4, %r10
    xorl %ebx, %ebx
.Li\@:
    cmpq \p, %rbx
    jge .Lend\@
    vxorpd %xmm1, %xmm1, %xmm1
    xorl %eax, %eax
.Lzv\@:
    cmpq %r10, %rax
    jge .Lzs\@
    vmovupd %ymm1, (%r13,%rax,8)
    addq $4, %rax
    jmp .Lzv\@
.Lzs\@:
    cmpq \q, %rax
    jge .Lk0\@
    vmovsd %xmm1, (%r13,%rax,8)
    incq %rax
    jmp .Lzs\@
.Lk0\@:
    movq %r12, %r15
    xorl %ebp, %ebp
.Lk\@:
    cmpq \r, %rbp
    jge .Lnext\@
    vbroadcastsd (%r14,%rbp,8), %ymm0
    xorl %eax, %eax
.Lv\@:
    cmpq %r10, %rax
    jge .Ls\@
    vmovupd (%r15,%rax,8), %ymm1
    vmulpd %ymm0, %ymm1, %ymm1
    vaddpd (%r13,%rax,8), %ymm1, %ymm1
    vmovupd %ymm1, (%r13,%rax,8)
    addq $4, %rax
    jmp .Lv\@
.Ls\@:
    cmpq \q, %rax
    jge .Lnextk\@
    vmovsd (%r15,%rax,8), %xmm1
    vmulsd %xmm0, %xmm1, %xmm1
    vaddsd (%r13,%rax,8), %xmm1, %xmm1
    vmovsd %xmm1, (%r13,%rax,8)
    incq %rax
    jmp .Ls\@
.Lnextk\@:
    leaq (%r15,\q,8), %r15
    incq %rbp
    jmp .Lk\@
.Lnext\@:
    leaq (%r13,\q,8), %r13
    leaq (%r14,\r,8), %r14
    incq %rbx
    jmp .Li\@
.Lend\@:
    .endm

    .text
    .globl mm3
    .type mm3, @function
mm3:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    PRODUCT %r9, 56(%rsp), 64(%rsp), %rdi, %rdx, %rsi
    PRODUCT 72(%rsp), 80(%rsp), 88(%rsp), %rsi, %r8, %rcx
    PRODUCT 96(%rsp), %r9, 72(%rsp), %rdi, %rsi, %rcx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    vzeroupper
    ret
    .size mm3, .-mm3
    .section .note.GNU-stack,"",@progbits
