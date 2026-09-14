.section .user, "a"
.global user_entry

user_entry:
    /* write(1, msg, len) */
    movl $4, %eax
    movl $1, %ebx
    movl $user_msg, %ecx
    movl $(user_msg_end - user_msg), %edx
    int $0x80

    /* exit(0) */
    movl $1, %eax
    xorl %ebx, %ebx
    int $0x80

    jmp .

user_msg:
    .ascii "[ring3] Hello from user mode!\n"
user_msg_end:
