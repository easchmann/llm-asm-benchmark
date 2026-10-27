    .text
    .globl sort8
    .type sort8, @function
sort8:
    shrq $3, %rdi
.Lgroup:
    testq %rdi, %rdi
    jle .Ldone
    movl $1, %ecx
.Louter:
    cmpq $8, %rcx
    jge .Lnext
    movq (%rsi,%rcx,8), %rax
    leaq -1(%rcx), %rdx
.Linner:
    testq %rdx, %rdx
    js .Lplace
    movq (%rsi,%rdx,8), %r8
    cmpq %rax, %r8
    jle .Lplace
    movq %r8, 8(%rsi,%rdx,8)
    decq %rdx
    jmp .Linner
.Lplace:
    movq %rax, 8(%rsi,%rdx,8)
    incq %rcx
    jmp .Louter
.Lnext:
    addq $64, %rsi
    decq %rdi
    jmp .Lgroup
.Ldone:
    ret
    .size sort8, .-sort8
    .section .note.GNU-stack,"",@progbits
