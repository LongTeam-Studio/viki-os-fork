.section .text
.global tss_flush

tss_flush:
    movw $0x28, %ax      /* GDT_TSS_SEL = 5 << 3 */
    ltr %ax
    ret
