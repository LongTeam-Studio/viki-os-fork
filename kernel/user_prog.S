.section .user, "awx"
.global user_entry

user_entry:
    /* 打印 prompt */
    movl $4, %eax
    movl $1, %ebx
    movl $prompt, %ecx
    movl $(prompt_end - prompt), %edx
    int $0x80

read_loop:
    /* read(0, ubuf, 1) */
    movl $3, %eax
    movl $0, %ebx
    movl $ubuf, %ecx
    movl $1, %edx
    int $0x80
    cmpl $0, %eax
    je read_loop

    /* 回显 */
    movl $4, %eax
    movl $1, %ebx
    movl $ubuf, %ecx
    movl $1, %edx
    int $0x80

    /* 回车则重打 prompt */
    movl $ubuf, %esi
    cmpb $0x0A, (%esi)
    jne read_loop

    movl $4, %eax
    movl $1, %ebx
    movl $prompt, %ecx
    movl $(prompt_end - prompt), %edx
    int $0x80
    jmp read_loop

prompt:
    .ascii "viki> "
prompt_end:

.align 4
ubuf:
    .byte 0
