#include "../include/ext2.h"
#include "../include/ata.h"
#include "../include/vga.h"

/* 全局超级块缓存 */
static ext2_superblock_t superblock;

int ext2_init(void) {
    uint16_t buffer[512]; /* 1024 字节 = 512 个 16-bit word */
    
    /* 超级块位于硬盘偏移 1024 字节处。
       一个扇区是 512 字节，所以偏移 1024 刚好是第 2 个扇区（LBA 2）。
       我们读 2 个扇区（1024字节）以确保读到完整的超级块。 */
    ata_read_sectors(2, 2, buffer);
    
    /* 把读到的数据强转为超级块结构体 */
    ext2_superblock_t *sb = (ext2_superblock_t *)buffer;
    
    /* 校验魔术字 0xEF53 */
    if (sb->s_magic == 0xEF53) {
        superblock = *sb; /* 拷贝到全局缓存 */
        vga_printf("EXT2: Filesystem detected!\n");
        vga_printf("  Inodes: %u, Blocks: %u\n", sb->s_inodes_count, sb->s_blocks_count);
        vga_printf("  Block size: %u bytes\n", 1024 << sb->s_log_block_size);
        return 0;
    } else {
        vga_printf("EXT2: Magic mismatch (0x%x), not an EXT2 disk.\n", sb->s_magic);
        return -1;
    }
}