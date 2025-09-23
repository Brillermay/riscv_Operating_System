#include "riscv.h"
#include "memlayout.h"
#include "printf.h"
#include "pmm.h"

/* 外部符号，由链接脚本定义 */
extern char end[]; // 内核代码和数据段的末尾

/* 空闲页链表 */
struct run {
    struct run *next;
};

struct {
    struct run *freelist;
} pmm;

/* 初始化物理内存管理器 */
void pmm_init() {
    printf("pmm_init: initializing physical memory manager...\n");
    
    /* 将内核末尾到物理内存顶部的所有内存逐页释放 */
    char *p = (char*)PGROUNDUP((uint64_t)end);
    for (; (uint64_t)p + PGSIZE <= PHYSTOP; p += PGSIZE) {
        free_page(p);
    }
    
    printf("pmm_init: initialization complete. Free memory starts at %p\n", pmm.freelist);
}

/* 释放一个物理页，将其加入空闲链表头部 */
void free_page(void *pa) {
    struct run *r;

    if (((uint64_t)pa % PGSIZE) != 0 || (char*)pa < end || (uint64_t)pa >= PHYSTOP) {
        printf("free_page: invalid physical address %p\n", pa);
        return;
    }

    /* 清空页面内容，便于调试 */
    char *p = (char*)pa;
    for(int i = 0; i < PGSIZE; i++) p[i] = 1;

    r = (struct run*)pa;
    r->next = pmm.freelist;
    pmm.freelist = r;
}

/* 分配一个物理页，从空闲链表头部取出一个 */
void* alloc_page(void) {
    struct run *r = pmm.freelist;

    if (r) {
        pmm.freelist = r->next;
    } else {
        printf("alloc_page: out of memory\n");
        return 0;
    }

    /* 清空页面内容 */
    char *p = (char*)r;
    for(int i = 0; i < PGSIZE; i++) p[i] = 0;

    return (void*)r;
}