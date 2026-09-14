#include "../include/tss.h"
#include "../include/gdt.h"

static struct tss_entry tss;

extern void tss_flush(void);

void tss_init(uint32_t kernel_stack_top) {
    uint8_t *p = (uint8_t *)&tss;
    for (unsigned i = 0; i < sizeof(tss); i++) p[i] = 0;

    tss.ss0  = GDT_KERNEL_DATA_SEL;
    tss.esp0 = kernel_stack_top;
    tss.iomap_base = sizeof(tss);

    /* TSS 描述符占两个 GDT 槽（16 字节）。当前 GDT_ENTRIES=6，
       第 5 槽是第二个 slot，需要把 GDT 扩到 7 才能装下。 */

    /* 低 8 字节：base[0:23] + limit[0:15] + access + limit[16:19] + flags */
    gdt_set_gate_raw(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

    /* 高 8 字节：base[24:31] 放在 byte 0，其余 0 */
    gdt_set_gate_raw_high(6, ((uint32_t)&tss) >> 24);

    tss_flush();
}

void tss_set_esp0(uint32_t esp0) {
    tss.esp0 = esp0;
}
