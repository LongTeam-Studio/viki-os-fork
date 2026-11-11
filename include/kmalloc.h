#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>

/*
 * 内核堆内存分配器
 * 设计思路：基于 PMM 页分配器实现简化版的 Slab 分配器。
 * 对于 <= 1024 字节的小对象，按 16 字节对齐划分固定缓存池。
 * 对于大对象，直接分配物理页。
 */

void kmalloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
