#include <stdint.h>
#include "../include/kmalloc.h"
#include "../include/pmm.h"
#include "../include/vga.h"
#include "../include/panic.h"

#define KMALLOC_MAGIC 0x4B4D414C
#define BIG_OBJECT    0xFFFFFFFF

typedef struct kmalloc_header {
    uint32_t magic;
    uint32_t cache_idx;
    uint32_t size;
} kmalloc_header_t;

typedef struct free_node {
    struct free_node *next;
} free_node_t;

/* 
 * 极简版分配器：
 * 不搞大小分类了，小于 2048 字节的全部当作同一类，分配一页切 2 块。
 * 彻底避免循环变量引发的作用域/输入法乱码问题！
 */
static free_node_t *free_list = 0;

void kmalloc_init(void) {
    free_list = 0;
}

void *kmalloc(size_t size) {
    if (size == 0) return 0;

    /* 如果太大，直接分配一页 */
    if (size > 2048) {
        void *page = pmm_alloc_page();
        if (!page) return 0;
        kmalloc_header_t *hdr = (kmalloc_header_t *)page;
        hdr->magic = KMALLOC_MAGIC;
        hdr->cache_idx = BIG_OBJECT;
        hdr->size = size;
        return (void *)((uint8_t *)page + sizeof(kmalloc_header_t));
    }

    /* 如果没空闲块了，分配一页，切成两块挂上去 */
    if (!free_list) {
        void *page = pmm_alloc_page();
        if (!page) return 0;

        /* 第一块：头部在第0字节，数据紧跟其后 */
        kmalloc_header_t *hdr1 = (kmalloc_header_t *)page;
        hdr1->magic = KMALLOC_MAGIC;
        hdr1->cache_idx = 0;
        hdr1->size = 2048;
        
        /* 第二块：头部在第2048字节 */
        kmalloc_header_t *hdr2 = (kmalloc_header_t *)((uint8_t *)page + 2048);
        hdr2->magic = KMALLOC_MAGIC;
        hdr2->cache_idx = 0;
        hdr2->size = 2048;

        /* 把数据区挂到空闲链表 */
        free_node_t *node1 = (free_node_t *)((uint8_t *)hdr1 + sizeof(kmalloc_header_t));
        free_node_t *node2 = (free_node_t *)((uint8_t *)hdr2 + sizeof(kmalloc_header_t));

        node1->next = node2;
        node2->next = 0;
        free_list = node1;
    }

    /* 从链表摘一个节点 */
    free_node_t *ret = free_list;
    free_list = ret->next;
    return (void *)ret;
}

void kfree(void *ptr) {
    kmalloc_header_t *hdr;
    free_node_t *node;

    if (!ptr) return;

    hdr = (kmalloc_header_t *)((uint8_t *)ptr - sizeof(kmalloc_header_t));

    if (hdr->magic != KMALLOC_MAGIC) {
        panic("kfree: bad magic");
    }

    if (hdr->cache_idx == BIG_OBJECT) {
        pmm_free_page(hdr);
        return;
    }

    node = (free_node_t *)ptr;
    node->next = free_list;
    free_list = node;
}
