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
        case SYS_write: {
            /* write(fd, buf, len) —— 只支持 fd=1 走 VGA */
            uint32_t fd  = regs->ebx;
            const char *buf = (const char *)regs->ecx;
            uint32_t len = regs->edx;
            (void)fd;  /* 第一版忽略 fd */
            for (uint32_t i = 0; i < len; i++) {
                vga_putc(buf[i]);
            }
            regs->eax = len;
            break;
        }
        //case SYS_exit:
            //vga_printf("[syscall] exit(%u)\n", regs->ebx);
            /* ring3 还没跳，先死循环 */
            //while (1) { __asm__ volatile ("hlt"); }
            //break;
        case SYS_exit:
            vga_printf("[syscall] exit(%u)\n", regs->ebx);
            vga_puts("System halted.\n");
            while (1) { __asm__ volatile ("hlt"); }
            break;

        case SYS_getpid:
            regs->eax = 1;
            break;

        default:
            vga_printf("[syscall] unknown nr=%u\n", nr);
            regs->eax = (uint32_t)-1;
            break;
    }
}

void syscall_init(void) {
    /* IDT 门已在 interrupt_init 里设过，这里只做日志 */
    vga_puts("Syscall subsystem initialized (int 0x80).\n");
}

/*
 * 内核态自测：直接构造 pt_regs 调 handler，不真发 int 0x80。
 * 验证分发逻辑，不用切 ring3。
 */
void syscall_selftest(void) {
    struct pt_regs r;
    /* 清零 */
    uint8_t *p = (uint8_t *)&r;
    for (unsigned i = 0; i < sizeof(r); i++) p[i] = 0;

    const char msg[] = "[selftest] hello via syscall_write\n";
    r.eax = SYS_write;
    r.ebx = 1;
    r.ecx = (uint32_t)msg;
    r.edx = sizeof(msg) - 1;

    syscall_handler(&r);
    vga_printf("[selftest] write returned %u\n", r.eax);
}
