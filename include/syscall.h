#ifndef SYSCALL_H
#define SYSCALL_H

#include "interrupt.h"

/* 系统调用号 */
#define SYS_exit   1
#define SYS_write  4
#define SYS_getpid 20

void syscall_init(void);
void syscall_handler(struct pt_regs *regs);

/* 内核态测试：手动构造 pt_regs 调 syscall_handler */
void syscall_selftest(void);

#endif
