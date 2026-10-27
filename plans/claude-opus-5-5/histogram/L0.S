    .text
    .globl histogram
    .type histogram, @function
histogram:
    xorl %eax, %eax
.Lclear:
    cmpq %rdx, %rax
    jge .Lcount0
    movq $0, (%rcx,%rax,8)
    incq %rax
    jmp .Lclear
.Lcount0:
    xorl %eax, %eax
.Lcount:
    cmpq %rdi, %rax
    jge .Ldone
    movq (%rsi,%rax,8), %r8
    incq (%rcx,%r8,8)
    incq %rax
    jmp .Lcount
.Ldone:
    ret
    .size histogram, .-histogram
    .section .note.GNU-stack,"",@progbits
