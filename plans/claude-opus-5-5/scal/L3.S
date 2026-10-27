    .text
    .globl scal
    .type scal, @function
scal:
    vbroadcastsd %xmm0, %ymm0
    movq %rdi, %rax
    andq $-4, %rax
    xorl %ecx, %ecx
.Lvec:
    cmpq %rax, %rcx
    jge .Ltail
    vmulpd (%rsi,%rcx,8), %ymm0, %ymm1
    vmovupd %ymm1, (%rsi,%rcx,8)
    addq $4, %rcx
    jmp .Lvec
.Ltail:
    cmpq %rdi, %rcx
    jge .Ldone
    vmulsd (%rsi,%rcx,8), %xmm0, %xmm1
    vmovsd %xmm1, (%rsi,%rcx,8)
    incq %rcx
    jmp .Ltail
.Ldone:
    vzeroupper
    ret
    .size scal, .-scal
    .section .note.GNU-stack,"",@progbits
