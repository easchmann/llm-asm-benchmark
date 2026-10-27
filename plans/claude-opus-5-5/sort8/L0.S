    .text
    .globl sort8
    .type sort8, @function
sort8:
    pushq %rbx
    pushq %rbp
    shrq $3, %rdi
.Lgroup:
    testq %rdi, %rdi
    jle .Ldone
    movq 0(%rsi), %rax
    movq 8(%rsi), %rcx
    movq 16(%rsi), %rdx
    movq 24(%rsi), %r8
    movq 32(%rsi), %r9
    movq 40(%rsi), %r10
    movq 48(%rsi), %r11
    movq 56(%rsi), %rbx
    movq %rax, %rbp
    cmpq %rcx, %rax
    cmovgq %rcx, %rax
    cmovgq %rbp, %rcx
    movq %rdx, %rbp
    cmpq %r8, %rdx
    cmovgq %r8, %rdx
    cmovgq %rbp, %r8
    movq %r9, %rbp
    cmpq %r10, %r9
    cmovgq %r10, %r9
    cmovgq %rbp, %r10
    movq %r11, %rbp
    cmpq %rbx, %r11
    cmovgq %rbx, %r11
    cmovgq %rbp, %rbx
    movq %rax, %rbp
    cmpq %rdx, %rax
    cmovgq %rdx, %rax
    cmovgq %rbp, %rdx
    movq %rcx, %rbp
    cmpq %r8, %rcx
    cmovgq %r8, %rcx
    cmovgq %rbp, %r8
    movq %r9, %rbp
    cmpq %r11, %r9
    cmovgq %r11, %r9
    cmovgq %rbp, %r11
    movq %r10, %rbp
    cmpq %rbx, %r10
    cmovgq %rbx, %r10
    cmovgq %rbp, %rbx
    movq %rcx, %rbp
    cmpq %rdx, %rcx
    cmovgq %rdx, %rcx
    cmovgq %rbp, %rdx
    movq %r10, %rbp
    cmpq %r11, %r10
    cmovgq %r11, %r10
    cmovgq %rbp, %r11
    movq %rax, %rbp
    cmpq %r9, %rax
    cmovgq %r9, %rax
    cmovgq %rbp, %r9
    movq %r8, %rbp
    cmpq %rbx, %r8
    cmovgq %rbx, %r8
    cmovgq %rbp, %rbx
    movq %rcx, %rbp
    cmpq %r10, %rcx
    cmovgq %r10, %rcx
    cmovgq %rbp, %r10
    movq %rdx, %rbp
    cmpq %r11, %rdx
    cmovgq %r11, %rdx
    cmovgq %rbp, %r11
    movq %rcx, %rbp
    cmpq %r9, %rcx
    cmovgq %r9, %rcx
    cmovgq %rbp, %r9
    movq %r8, %rbp
    cmpq %r11, %r8
    cmovgq %r11, %r8
    cmovgq %rbp, %r11
    movq %rdx, %rbp
    cmpq %r9, %rdx
    cmovgq %r9, %rdx
    cmovgq %rbp, %r9
    movq %r8, %rbp
    cmpq %r10, %r8
    cmovgq %r10, %r8
    cmovgq %rbp, %r10
    movq %r8, %rbp
    cmpq %r9, %r8
    cmovgq %r9, %r8
    cmovgq %rbp, %r9
    movq %rax, 0(%rsi)
    movq %rcx, 8(%rsi)
    movq %rdx, 16(%rsi)
    movq %r8, 24(%rsi)
    movq %r9, 32(%rsi)
    movq %r10, 40(%rsi)
    movq %r11, 48(%rsi)
    movq %rbx, 56(%rsi)
    addq $64, %rsi
    decq %rdi
    jmp .Lgroup
.Ldone:
    popq %rbp
    popq %rbx
    ret
    .size sort8, .-sort8
    .section .note.GNU-stack,"",@progbits
