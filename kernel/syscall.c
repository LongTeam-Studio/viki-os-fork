#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/interrupt.h"

void syscall_handler(struct pt_regs *regs) {
    uint32_t nr = regs->eax;
    vga_printf("[syscall] nr=%u\n", nr);

    switch (nr) {
        case 1:
            vga_printf("[syscall] exit(%u)\n", regs->ebx);
            vga_puts("System halted.\n");
            while (1) { __asm__ volatile ("hlt"); }
            break;
        case 4:
            {
                const char *buf = (const char *)regs->ecx;
                uint32_t len = regs->edx;
                for (uint32_t i = 0; i < len; i++) vga_putc(buf[i]);
                regs->eax = len;
            }
            break;
        case 20:
            regs->eax = 1;
            break;
        default:
            break;
    }
}
void syscall_init(void) {}
