    .text
    .globl lower_bound
    .type lower_bound, @function
lower_bound:
.Lkey:
    testq %rdx, %rdx
    jle .Ldone
    movq (%rcx), %r10
    xorl %r11d, %r11d
    movq %rdi, %rax
.Lsearch:
    cmpq %rax, %r11
    jge .Lstore
    leaq (%r11,%rax), %r9
    shrq $1, %r9
    cmpq %r10, (%rsi,%r9,8)
    jge .Lhigh
    leaq 1(%r9), %r11
    jmp .Lsearch
.Lhigh:
    movq %r9, %rax
    jmp .Lsearch
.Lstore:
    movq %r11, (%r8)
    addq $8, %rcx
    addq $8, %r8
    decq %rdx
    jmp .Lkey
.Ldone:
    ret
    .size lower_bound, .-lower_bound
    .section .note.GNU-stack,"",@progbits
