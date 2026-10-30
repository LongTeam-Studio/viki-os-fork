#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/interrupt.h"

/*
 * 系统调用分发
 * 参数约定（Linux i386 风格）：
 *   eax = syscall 号
 *   ebx, ecx, edx, esi, edi, ebp = 参数 1..6
 * 返回值放回 eax
 */
void syscall_handler(struct pt_regs *regs) {
    uint32_t nr = regs->eax;

    switch (nr) {
        case 1: /* SYS_exit */
            vga_printf("[syscall] exit(%u)\n", regs->ebx);
            vga_puts("System halted.\n");
            while (1) { __asm__ volatile ("hlt"); }
            break;

        case 4: /* SYS_write */
            {
                const char *buf = (const char *)regs->ecx;
                uint32_t len = regs->edx;
                for (uint32_t i = 0; i < len; i++) {
                    vga_putc(buf[i]);
                }
                regs->eax = len;
            }
            break;

        case 20: /* SYS_getpid */
            regs->eax = 1;
            break;

        default:
            vga_printf("[syscall] unknown nr=%u\n", nr);
            regs->eax = (uint32_t)-1;
            break;
    }
}

void syscall_init(void) {
    /* IDT 门已经在 interrupt_init 里注册 */
}
