#include "riscv.h"
#include "memlayout.h"
#include "printf.h"
#include "pmm.h"
#include "vmm.h"

/* 外部符号 */
extern char etext[]; // 内核代码段结束地址

/* 内核页表 */
pagetable_t kernel_pagetable;

/* 遍历页表，查找给定虚拟地址对应的PTE。如果不存在且alloc为1，则创建。*/
static pte_t* walk(pagetable_t pagetable, uint64_t va, int alloc) {
    if (va >= (1L << 39)) { // Sv39虚拟地址不能超过39位
        return 0;
    }

    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[VPN(va, level)];
        if (*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if (!alloc || (pagetable = (pagetable_t)alloc_page()) == 0) {
                return 0;
            }
            // 清空新分配的页表页
            for(int i = 0; i < PGSIZE / sizeof(pte_t); i++) pagetable[i] = 0;
            *pte = PA2PTE((uint64_t)pagetable) | PTE_V; 
        }
    }
    return &pagetable[VPN(va, 0)];
}

/* 映射一个页面 */
static int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm) {
    if ((va % PGSIZE) != 0 || (pa % PGSIZE) != 0) {
        return -1; // 地址必须页对齐
    }

    pte_t *pte = walk(pagetable, va, 1);
    if (pte == 0) {
        return -1; // 内存不足
    }
    if (*pte & PTE_V) {
        printf("map_page: remap\n");
        return -1; // 已被映射
    }
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}

/* 映射一个内存区域 */
int map_region(pagetable_t pt, uint64_t va, uint64_t pa, uint64_t size, int perm) {
    uint64_t a, last;

    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);

    for (;;) {
        if (map_page(pt, a, pa, perm) != 0) {
            return -1;
        }
        if (a == last) {
            break;
        }
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

/* 创建一个空的页表 */
pagetable_t create_pagetable(void) {
    pagetable_t pagetable = (pagetable_t)alloc_page();
    if (pagetable == 0) return 0;
    for(int i = 0; i < PGSIZE / sizeof(pte_t); i++) pagetable[i] = 0;
    return pagetable;
}

/* 创建内核页表 */
void kvminit(void) {
    printf("kvminit: creating kernel page table...\n");
    kernel_pagetable = create_pagetable();
    if (kernel_pagetable == 0) {
        printf("kvminit: failed to create kernel page table\n");
        return;
    }

    // 映射UART设备
    printf("kvminit: mapping UART...\n");
    map_region(kernel_pagetable, UART0, UART0, PGSIZE, PTE_R | PTE_W);

    // 映射内核代码段 (R+X)
    printf("kvminit: mapping kernel text...\n");
    /* 确保映射大小按页对齐，覆盖到 etext 所在页 */
    uint64_t text_start = KERNBASE;
    uint64_t text_end = (uint64_t)etext;
    uint64_t text_size = 0;
    if (text_end > text_start) {
        text_size = PGROUNDUP(text_end - text_start);
    }
    if (text_size > 0) {
        if (map_region(kernel_pagetable, text_start, KERNBASE, text_size, PTE_R | PTE_X) != 0) {
            printf("kvminit: failed to map kernel text\n");
            return;
        }
    }

    // 映射内核数据段和剩余物理内存 (R+W)
    printf("kvminit: mapping kernel data and physical memory...\n");
    /* 数据段从 etext 向上取页对齐开始 */
    uint64_t data_pa = PGROUNDUP(text_end);
    uint64_t data_va = data_pa;
    if (data_pa < PHYSTOP) {
        uint64_t data_size = PHYSTOP - data_pa;
        if (map_region(kernel_pagetable, data_va, data_pa, data_size, PTE_R | PTE_W) != 0) {
            printf("kvminit: failed to map kernel data/phys memory\n");
            return;
        }
    }

    printf("kvminit: kernel page table created.\n");
}

/* 激活内核页表 */
void kvminithart(void) {
    printf("kvminithart: activating kernel page table...\n");
    w_satp((((uint64_t)kernel_pagetable) >> 12) | (8L << 60));
    sfence_vma();
    printf("kvminithart: paging enabled.\n");
}