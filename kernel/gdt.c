#include "../include/gdt.h"

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdt_ptr;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= granularity & 0xF0;

    gdt[num].access = access;
}

void gdt_set_gate_raw(int num, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity) {
    gdt_set_gate(num, base, limit, access, granularity);
}

void gdt_set_gate_raw_high(int num, uint8_t base_high) {
    uint8_t *p = (uint8_t *)&gdt[num];
    for (int i = 0; i < 8; i++) p[i] = 0;
    p[0] = base_high;
}

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);

    gdt_set_gate(GDT_KERNEL_CODE, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    gdt_set_gate(GDT_KERNEL_DATA, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_set_gate(GDT_USER_CODE, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    gdt_set_gate(GDT_USER_DATA, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gdt_ptr);
}
