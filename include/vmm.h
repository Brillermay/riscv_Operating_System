#ifndef VMM_H
#define VMM_H

#include "riscv.h"

/* 初始化内核页表 */
void kvminit(void);

/* 激活内核页表 */
void kvminithart(void);

/* 创建一个空的页表 */
pagetable_t create_pagetable(void);

/* 映射一个区域 */
int map_region(pagetable_t pt, uint64_t va, uint64_t pa, uint64_t size, int perm);

#endif