#ifndef PMM_H
#define PMM_H

#include <stdint.h>

/* 初始化物理内存管理器 */
void pmm_init(void);

/* 分配一个物理页 */
void* alloc_page(void);

/* 释放一个物理页 */
void free_page(void* pa);

#endif