.section .user, "awx"
.global user_entry

.set SYS_EXIT,  1
.set SYS_READ,  3
.set SYS_WRITE, 4

user_entry:
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $welcome, %ecx
    movl $(welcome_end - welcome), %edx
    int $0x80

main_loop:
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $prompt, %ecx
    movl $(prompt_end - prompt), %edx
    int $0x80

    call read_line

    movl $linebuf, %esi
skip_lead:
    movzbl (%esi), %eax
    cmpl $0x20, %eax
    jne check_empty
    incl %esi
    jmp skip_lead

check_empty:
    cmpl $0, %eax
    je main_loop

    movl %esi, %edi
find_end:
    movzbl (%edi), %eax
    cmpl $0, %eax
    je got_cmd
    cmpl $0x20, %eax
    je split
    incl %edi
    jmp find_end
split:
    movb $0, (%edi)
    incl %edi
    movl %edi, argptr
    jmp have_args
got_cmd:
    movl %edi, argptr

have_args:
    movl %esi, %eax
    movl $s_help, %edx
    call scmp
    testl %eax, %eax
    jz cmd_help

    movl %esi, %eax
    movl $s_echo, %edx
    call scmp
    testl %eax, %eax
    jz cmd_echo

    movl %esi, %eax
    movl $s_exit, %edx
    call scmp
    testl %eax, %eax
    jz cmd_exit

    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $msg_unknown, %ecx
    movl $(msg_unknown_end - msg_unknown), %edx
    int $0x80

    movl %esi, %ecx
    call slen
    movl %eax, %edx
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    int $0x80

    call print_nl
    jmp main_loop

cmd_help:
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $msg_help, %ecx
    movl $(msg_help_end - msg_help), %edx
    int $0x80
    jmp main_loop

cmd_echo:
    movl argptr, %ecx
    cmpb $0, (%ecx)
    je .echo_nl
    call slen
    movl %eax, %edx
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    int $0x80
.echo_nl:
    call print_nl
    jmp main_loop

cmd_exit:
    movl $SYS_EXIT, %eax
    xorl %ebx, %ebx
    int $0x80
    jmp .

print_nl:
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $newline, %ecx
    movl $1, %edx
    int $0x80
    ret

slen:
    pushl %esi
    movl %ecx, %esi
    xorl %eax, %eax
.sl:
    cmpb $0, (%esi)
    je .sld
    incl %eax
    incl %esi
    jmp .sl
.sld:
    popl %esi
    ret

scmp:
    pushl %esi
    pushl %edi
    movl %eax, %esi
    movl %edx, %edi
.sc:
    movzbl (%esi), %eax
    movzbl (%edi), %ecx
    cmpl %ecx, %eax
    jne .scd
    testl %eax, %eax
    jz .scs
    incl %esi
    incl %edi
    jmp .sc
.scs:
    xorl %eax, %eax
    popl %edi
    popl %esi
    ret
.scd:
    subl %ecx, %eax
    popl %edi
    popl %esi
    ret

read_line:
    xorl %edi, %edi
.rl:
    movl $SYS_READ, %eax
    movl $0, %ebx
    movl $linebuf, %ecx
    addl %edi, %ecx
    movl $1, %edx
    int $0x80
    testl %eax, %eax
    jz .rl

    movzbl linebuf(%edi), %eax

    cmpl $0x0A, %eax
    je .rldone

    cmpl $0x08, %eax
    je .rlbs

    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $linebuf, %ecx
    addl %edi, %ecx
    movl $1, %edx
    int $0x80

    incl %edi
    cmpl $126, %edi
    jl .rl
    jmp .rldone

.rlbs:
    testl %edi, %edi
    jz .rl
    decl %edi
    movl $SYS_WRITE, %eax
    movl $1, %ebx
    movl $bs_seq, %ecx
    movl $3, %edx
    int $0x80
    jmp .rl

.rldone:
    call print_nl
    movb $0, linebuf(%edi)
    ret

.align 4
linebuf:
    .skip 128
argptr:
    .long 0

welcome:
    .ascii "VIKI shell (ring3). Type 'help'.\n"
welcome_end:

prompt:
    .ascii "viki> "
prompt_end:

s_help:
    .asciz "help"
s_echo:
    .asciz "echo"
s_exit:
    .asciz "exit"

msg_help:
    .ascii "commands:\n  help        - this\n  echo <text> - print text\n  exit        - halt\n"
msg_help_end:

msg_unknown:
    .ascii "unknown: "
msg_unknown_end:

newline:
    .byte 0x0A
bs_seq:
    .byte 0x08, 0x20, 0x08
