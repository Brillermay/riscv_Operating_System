#ifndef RISCV_H
#define RISCV_H

#include <stdint.h>

/* 页大小 (4KB) */
#define PGSIZE 4096
#define PGSHIFT 12

/* 向上/向下取整到页边界 */
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

/* 页表项 (PTE) 标志位 */
#define PTE_V (1L << 0) // 有效位
#define PTE_R (1L << 1) // 可读
#define PTE_W (1L << 2) // 可写
#define PTE_X (1L << 3) // 可执行
#define PTE_U (1L << 4) // 用户态可访问

/* 将PTE转换为物理地址 */
#define PTE2PA(pte) (((pte) >> 10) << 12)

/* 将物理地址转换为PTE */
#define PA2PTE(pa) (((pa) >> 12) << 10)

/* 从虚拟地址中提取各级VPN (Virtual Page Number) */
#define VPN(va, level) (((uint64_t)(va) >> (12 + 9 * (level))) & 0x1FF)

/* 定义页表和页表项类型 */
typedef uint64_t pte_t;
typedef uint64_t* pagetable_t;

/* 读写SATP寄存器 (Supervisor Address Translation and Protection) */
static inline void w_satp(uint64_t x) {
    asm volatile("csrw satp, %0" : : "r" (x));
}

static inline uint64_t r_satp(void) {
    uint64_t x;
    asm volatile("csrr %0, satp" : "=r" (x));
    return x;
}

/* 刷新TLB (Translation Lookaside Buffer) */
static inline void sfence_vma(void) {
    asm volatile("sfence.vma zero, zero");
}

#endif