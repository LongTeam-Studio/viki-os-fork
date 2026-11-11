#include "../include/ext2.h"
#include "../include/ata.h"
#include "../include/vga.h"

static ext2_superblock_t superblock;

/* 全局大缓冲区，绝对不能放在栈上，否则内和栈 4KB 直接爆掉 */
static uint16_t block_buffer[2048]; /* 4KB */

static void ext2_read_inode(uint32_t inode_num, ext2_inode_t *inode_out) {
    uint32_t block_size = 1024 << superblock.s_log_block_size;
    uint32_t bg_block = superblock.s_first_data_block + 1;
    uint32_t *bg;
    uint32_t inode_table_block;
    uint32_t inode_index;
    uint32_t inode_offset;
    uint32_t inode_block;
    uint32_t offset_in_block;
    uint8_t *ptr;
    uint32_t i;

    /* 读取 GDT (组描述符表) */
    ata_read_sectors(bg_block * (block_size / 512), 1, block_buffer);
    bg = (uint32_t *)block_buffer;
    inode_table_block = bg[2];

    vga_printf("DBG: first_data=%u, bg_blk=%u, itable=%u\n", 
               superblock.s_first_data_block, bg_block, inode_table_block);

    inode_index = inode_num - 1;
    inode_offset = inode_index * (superblock.s_inode_size ? superblock.s_inode_size : 128);
    inode_block = inode_table_block + (inode_offset / block_size);
    offset_in_block = inode_offset % block_size;

    /* 读取 Inode 所在块 (只读 1 个扇区就够找 128 字节了，避免多读报错) */
    ata_read_sectors(inode_block * (block_size / 512), 2, block_buffer);
    ptr = (uint8_t *)block_buffer + offset_in_block;
    for (i = 0; i < sizeof(ext2_inode_t); i++) {
        ((uint8_t *)inode_out)[i] = ptr[i];
    }
}

int ext2_init(void) {
    ext2_superblock_t *sb;
    ata_read_sectors(2, 2, block_buffer);
    sb = (ext2_superblock_t *)block_buffer;
    
    if (sb->s_magic == 0xEF53) {
        superblock = *sb;
        vga_printf("EXT2: Filesystem detected!\n");
        return 0;
    }
    vga_printf("EXT2: Magic mismatch!\n");
    return -1;
}

void ext2_list_root(void) {
    uint32_t block_size = 1024 << superblock.s_log_block_size;
    ext2_inode_t root_inode;
    uint32_t dir_block;
    uint8_t *ptr;
    uint32_t offset = 0;
    ext2_dir_entry_t *entry;
    char name[64];
    uint32_t name_len;
    uint32_t i;

    ext2_read_inode(2, &root_inode);
    vga_printf("DBG: i_size=%u, i_block0=%u\n", root_inode.i_size, root_inode.i_block[0]);
    vga_printf("EXT2: Files in /:\n");

    dir_block = root_inode.i_block[0];
    if (dir_block == 0) {
        vga_printf("  (Root block is 0, abort)\n");
        return;
    }

    /* 读取目录数据 */
    ata_read_sectors(dir_block * (block_size / 512), 2, block_buffer);
    ptr = (uint8_t *)block_buffer;
    
    while (offset < root_inode.i_size) {
        entry = (ext2_dir_entry_t *)(ptr + offset);
        if (entry->inode == 0 || entry->rec_len == 0) break;
        
        name_len = entry->name_len > 63 ? 63 : entry->name_len;
        for (i = 0; i < name_len; i++) {
            name[i] = entry->name[i];
        }
        name[name_len] = 0;

        vga_printf("  - %s (inode %u)\n", name, entry->inode);
        offset += entry->rec_len;
    }
}
