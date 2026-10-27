    .text
    .globl prefix_sum
    .type prefix_sum, @function
prefix_sum:
    xorl %eax, %eax
    xorl %ecx, %ecx
.Lloop:
    cmpq %rdi, %rcx
    jge .Ldone
    movq (%rsi,%rcx,8), %r8
    addq %r8, %rax
    movq %rax, (%rdx,%rcx,8)
    incq %rcx
    jmp .Lloop
.Ldone:
    ret
    .size prefix_sum, .-prefix_sum
    .section .note.GNU-stack,"",@progbits
