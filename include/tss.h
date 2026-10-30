#ifndef TSS_H
#define TSS_H

#include <stdint.h>

/*
 * 32 位 TSS 结构（Intel SDM Vol.3 Figure 7-2）
 * 共 104 字节，iomap_base 之后是 I/O 权限位图，这里不用，直接指向结构末尾。
 */
struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;      /* ring3 -> ring0 时 CPU 自动加载的栈指针 */
    uint32_t ss0;       /* 内核数据段选择子 */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

void tss_init(uint32_t kernel_stack_top);
void tss_set_esp0(uint32_t esp0);

#endif
