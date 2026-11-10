#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

/*
 * EXT2 超级块结构体
 * 位于硬盘第 1024 字节偏移处，长度 1024 字节。
 * 这里只列出最关键的几个字段用于验证。
 */
typedef struct {
    uint32_t s_inodes_count;        /* 0x00: inode 总数 */
    uint32_t s_blocks_count;        /* 0x04: 块总数 */
    uint32_t s_r_blocks_count;      /* 0x08: 保留块数 */
    uint32_t s_free_blocks_count;   /* 0x0C: 空闲块数 */
    uint32_t s_free_inodes_count;   /* 0x10: 空闲 inode 数 */
    uint32_t s_first_data_block;    /* 0x14: 第一个数据块号 */
    uint32_t s_log_block_size;      /* 0x18: 块大小 (0=1KB, 1=2KB, 2=4KB) */
    uint32_t s_log_frag_size;       /* 0x1C: 碎片大小 */
    uint32_t s_blocks_per_group;    /* 0x20: 每组块数 */
    uint32_t s_frags_per_group;     /* 0x24: 每组碎片数 */
    uint32_t s_inodes_per_group;    /* 0x28: 每组 inode 数 */
    uint32_t s_mtime;               /* 0x2C: 挂载时间 */
    uint32_t s_wtime;               /* 0x30: 写入时间 */
    uint16_t s_mnt_count;           /* 0x34: 挂载次数 */
    uint16_t s_max_mnt_count;       /* 0x36: 最大挂载次数 */
    uint16_t s_magic;               /* 0x38: 魔术字 (必须是 0xEF53) */
    uint16_t s_state;               /* 0x3A: 文件系统状态 */
} __attribute__((packed)) ext2_superblock_t;

/* 初始化 EXT2 文件系统，返回 0 表示成功，-1 表示失败 */
int ext2_init(void);

#endif