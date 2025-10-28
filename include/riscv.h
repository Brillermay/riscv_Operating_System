#ifndef RISCV_H
#define RISCV_H

#include <stdint.h>

/* ====== 原有：分页/页表工具 ====== */

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

/* ====== 新增：CSR/中断/定时器/CLINT 工具 ====== */

/* CSR 访问 */
static inline uint64_t r_mstatus(void){ uint64_t x; asm volatile("csrr %0, mstatus":"=r"(x)); return x; }
static inline void     w_mstatus(uint64_t x){ asm volatile("csrw mstatus, %0"::"r"(x)); }
static inline uint64_t r_mie(void){ uint64_t x; asm volatile("csrr %0, mie":"=r"(x)); return x; }
static inline void     w_mie(uint64_t x){ asm volatile("csrw mie, %0"::"r"(x)); }
static inline uint64_t r_mip(void){ uint64_t x; asm volatile("csrr %0, mip":"=r"(x)); return x; }
static inline void     w_mip(uint64_t x){ asm volatile("csrw mip, %0"::"r"(x)); }
static inline void     w_mtvec(uint64_t x){ asm volatile("csrw mtvec, %0"::"r"(x)); }
static inline uint64_t r_mcause(void){ uint64_t x; asm volatile("csrr %0, mcause":"=r"(x)); return x; }
static inline uint64_t r_mepc(void){ uint64_t x; asm volatile("csrr %0, mepc":"=r"(x)); return x; }
static inline void     w_mepc(uint64_t x){ asm volatile("csrw mepc, %0"::"r"(x)); }
static inline uint64_t r_mtval(void){ uint64_t x; asm volatile("csrr %0, mtval":"=r"(x)); return x; }
static inline uint64_t r_mhartid(void){ uint64_t x; asm volatile("csrr %0, mhartid":"=r"(x)); return x; }

/* 置/清 mstatus/mie 位 */
static inline void set_mstatus(uint64_t mask){ w_mstatus(r_mstatus() | mask); }
static inline void clr_mstatus(uint64_t mask){ w_mstatus(r_mstatus() & ~mask); }
static inline void set_mie(uint64_t mask){ w_mie(r_mie() | mask); }
static inline void clr_mie(uint64_t mask){ w_mie(r_mie() & ~mask); }

/* mstatus/mie 位掩码 */
#define MSTATUS_MIE (1ULL << 3)   /* 全局M态中断使能 */
#define MIE_MSIE    (1ULL << 3)   /* 机器软件中断 */
#define MIE_MTIE    (1ULL << 7)   /* 机器定时器中断 */
#define MIE_MEIE    (1ULL << 11)  /* 机器外部中断 */

/* QEMU virt 平台 CLINT 基址与寄存器 */
#define CLINT_BASE         0x02000000ULL
#define CLINT_MTIMECMP(h) (CLINT_BASE + 0x4000ULL + 8ULL*(h))
#define CLINT_MTIME       (CLINT_BASE + 0xBFF8ULL)

/* 读写CLINT 64位寄存器 */
static inline uint64_t clint_read64(uint64_t addr){
    volatile uint64_t *p = (volatile uint64_t*)addr;
    return *p;
}
static inline void clint_write64(uint64_t addr, uint64_t val){
    volatile uint64_t *p = (volatile uint64_t*)addr;
    *p = val;
}

/* 读写 mideleg/medeleg（用来在 M-mode 委托给 S-mode） */
static inline uint64_t r_mideleg(void){ uint64_t x; asm volatile("csrr %0, mideleg":"=r"(x)); return x; }
static inline void     w_mideleg(uint64_t x){ asm volatile("csrw mideleg, %0"::"r"(x)); }
static inline uint64_t r_medeleg(void){ uint64_t x; asm volatile("csrr %0, medeleg":"=r"(x)); return x; }
static inline void     w_medeleg(uint64_t x){ asm volatile("csrw medeleg, %0"::"r"(x)); }

/* 读写 stvec/scause/sepc/stval 的 wrapper（S-mode trap 处理会用到） */
static inline void     w_stvec(uint64_t x){ asm volatile("csrw stvec, %0"::"r"(x)); }
static inline uint64_t r_scause(void){ uint64_t x; asm volatile("csrr %0, scause":"=r"(x)); return x; }
static inline uint64_t r_sepc(void){ uint64_t x; asm volatile("csrr %0, sepc":"=r"(x)); return x; }
static inline uint64_t r_stval(void){ uint64_t x; asm volatile("csrr %0, stval":"=r"(x)); return x; }

/* 新增：sscratch/sstatus/sie 等封装 */
static inline void     w_sscratch(uint64_t x){ asm volatile("csrw sscratch, %0"::"r"(x)); }
static inline uint64_t r_sscratch(void){ uint64_t x; asm volatile("csrr %0, sscratch":"=r"(x)); return x; }
static inline void     w_sstatus(uint64_t x){ asm volatile("csrw sstatus, %0"::"r"(x)); }
static inline uint64_t r_sstatus(void){ uint64_t x; asm volatile("csrr %0, sstatus":"=r"(x)); return x; }
static inline void     w_sie(uint64_t x){ asm volatile("csrw sie, %0"::"r"(x)); }
static inline uint64_t r_sie(void){ uint64_t x; asm volatile("csrr %0, sie":"=r"(x)); return x; }

/* 定义：mideleg 的 STIP 位（按特权规范，STIP 通常是中断号 5，对应位 5） */
#define MIDELEG_STIP (1ULL << 5)

/* 新增：mstatus MPP 字段相关定义（用于 mret 返回时选择目标特权级） */
#define MSTATUS_MPP_SHIFT 11
#define MSTATUS_MPP_MASK  (3ULL << MSTATUS_MPP_SHIFT)
#define MSTATUS_MPP_S     (1ULL << MSTATUS_MPP_SHIFT)

/* 新增：sstatus SIE 位（Supervisor Interrupt Enable） */
#define SSTATUS_SIE (1ULL << 1)

#endif