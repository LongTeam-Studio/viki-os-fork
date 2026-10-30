.section .text
.global enter_ring3

/*
 * void enter_ring3(uint32_t entry, uint32_t stack_top)
 *  4(%esp) = entry     用户态入口地址
 *  8(%esp) = stack_top 用户栈顶
 */
enter_ring3:
    movl 4(%esp), %ecx
    movl 8(%esp), %edx

    /* 切换到用户数据段（0x23 = user data | RPL3） */
    movw $0x23, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    /* 构造 iret 帧：SS, ESP, EFLAGS, CS, EIP */
    pushl $0x23
    pushl %edx
    pushfl
    orl $0x200, (%esp)
    pushl $0x1B
    pushl %ecx
    iret
