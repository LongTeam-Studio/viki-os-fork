#include "../include/ata.h"
#include "../include/port.h"
#include "../include/vga.h"

/* ATA 主通道 I/O 端口 */
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_STATUS_CMD   0x1F7
#define ATA_PRIMARY_ALT_STATUS   0x3F6

#define ATA_SR_BSY     0x80    /* 忙 */
#define ATA_SR_DRDY    0x40    /* 设备就绪 */
#define ATA_SR_DF      0x20    /* 设备故障 */
#define ATA_SR_DSC     0x10    /* 设备寻道完成 */
#define ATA_SR_DRQ     0x08    /* 数据请求就绪 */
#define ATA_SR_CORR    0x04    /* 数据纠正 */
#define ATA_SR_IDX     0x02    /* 索引 */
#define ATA_SR_ERR     0x01    /* 错误 */

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_IDENTIFY 0xEC

static int drive_present = 0;

/* 等待 BSY 清零 */
static void ata_wait_bsy(void) {
    while (inb(ATA_PRIMARY_STATUS_CMD) & ATA_SR_BSY);
}

/* 等待 DRQ 置位，准备传输数据 */
static void ata_wait_drq(void) {
    while (!(inb(ATA_PRIMARY_STATUS_CMD) & ATA_SR_DRQ));
}

void ata_init(void) {
    /* 选中主盘 */
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xA0);
    
    /* 发送 IDENTIFY 命令 */
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_LBA_LO, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HI, 0);
    outb(ATA_PRIMARY_STATUS_CMD, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_PRIMARY_STATUS_CMD);
    if (status == 0) {
        drive_present = 0;
        vga_puts("ATA: No drive found.\n");
        return;
    }

    ata_wait_bsy();
    
    if (inb(ATA_PRIMARY_LBA_MID) != 0 || inb(ATA_PRIMARY_LBA_HI) != 0) {
        drive_present = 0;
        vga_puts("ATA: Not an ATA device.\n");
        return;
    }

    ata_wait_drq();

    /* 读走 256 字节的识别数据，清空缓冲区 */
    uint16_t i;
    for (i = 0; i < 256; i++) {
        inb(ATA_PRIMARY_DATA);
        inb(ATA_PRIMARY_DATA + 1); /* 16-bit read */
    }

    drive_present = 1;
    vga_puts("ATA: Primary Master drive initialized.\n");
}

int ata_drive_present(void) {
    return drive_present;
}

void ata_read_sectors(uint32_t lba, uint8_t count, uint16_t *buffer) {
    if (!drive_present) return;

    ata_wait_bsy();

    /* 设置读取的扇区数和 LBA 地址 */
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, count);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    
    /* 发送读命令 */
    outb(ATA_PRIMARY_STATUS_CMD, ATA_CMD_READ_PIO);

    /* 循环读取所有扇区 */
    uint32_t i, j;
    for (i = 0; i < count; i++) {
        ata_wait_bsy();
        ata_wait_drq();
        
        /* 一个扇区 512 字节 = 256 个 16-bit word */
        for (j = 0; j < 256; j++) {
            buffer[i * 256 + j] = inw(ATA_PRIMARY_DATA);
        }
    }
}