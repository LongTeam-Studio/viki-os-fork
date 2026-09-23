#include "../include/panic.h"
#include "../include/vga.h"

/*
 * panic - 内核致命错误
 *
 * 流程：
 *   1. cli 关中断，避免打印过程被 IRQ 打断
 *   2. 打印醒目头部 + 原因
 *   3. hlt 死循环，等外部重启
 *
 * 不做栈回溯（还没实现），不 dump 寄存器（无意义，寄存器已丢）。
 */
void panic(const char *msg) {
    __asm__ volatile ("cli");

    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts(msg);
    vga_puts("\n\nSystem halted.\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
