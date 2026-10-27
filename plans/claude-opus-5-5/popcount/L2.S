    .text
    .globl popcount
    .type popcount, @function
popcount:
    movabsq $0x5555555555555555, %r8
    movabsq $0x3333333333333333, %r9
    movabsq $0x0F0F0F0F0F0F0F0F, %r10
    movabsq $0x0101010101010101, %r11
.Lloop:
    testq %rdi, %rdi
    jle .Ldone
    movq (%rsi), %rax
    movq %rax, %rcx
    shrq $1, %rcx
    andq %r8, %rcx
    subq %rcx, %rax
    movq %rax, %rcx
    shrq $2, %rcx
    andq %r9, %rcx
    andq %r9, %rax
    addq %rcx, %rax
    movq %rax, %rcx
    shrq $4, %rcx
    addq %rcx, %rax
    andq %r10, %rax
    imulq %r11, %rax
    shrq $56, %rax
    movq %rax, (%rdx)
    addq $8, %rsi
    addq $8, %rdx
    decq %rdi
    jmp .Lloop
.Ldone:
    ret
    .size popcount, .-popcount
    .section .note.GNU-stack,"",@progbits
