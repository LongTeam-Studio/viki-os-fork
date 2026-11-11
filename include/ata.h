#ifndef ATA_H
#define ATA_H

#include <stdint.h>

/* 初始化 ATA 硬盘 (目前只处理 Primary Master) */
void ata_init(void);

/* 从硬盘读取扇区
 * lba: 逻辑块地址 (0 代表第一个扇区)
 * count: 读取的扇区数量 (每个扇区 512 字节)
 * buffer: 读入数据的内存指针
 */
void ata_read_sectors(uint32_t lba, uint8_t count, uint16_t *buffer);

/* 获取硬盘是否存在 */
int ata_drive_present(void);

#endif