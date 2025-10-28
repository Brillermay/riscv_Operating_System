// filepath: /home/bri/Desktop/riscv-os/riscv_Operating_System/kernel/main.c
#include "printf.h"
#include "pmm.h"
#include "vmm.h"
#include "uart.h"
#include "trap.h"   // 新增：启用中断初始化

void test_physical_memory(void) {
    printf("\n=== Testing Physical Memory Allocator ===\n");
    void *p1 = alloc_page();
    printf("Allocated page 1 at %p\n", p1);
    void *p2 = alloc_page();
    printf("Allocated page 2 at %p\n", p2);
    
    if(p1 != 0 && p2 != 0) {
        printf("Allocation successful.\n");
    } else {
        printf_color(COLOR_RED, "Allocation failed!\n");
    }

    free_page(p1);
    printf("Freed page 1.\n");
    free_page(p2);
    printf("Freed page 2.\n");

    void *p3 = alloc_page();
    printf("Allocated page 3 at %p (should be same as last freed page).\n", p3);
    free_page(p3);
    printf("PMM test complete.\n");
}

void main(void) {
    /* 初始化UART和printf */
    uart_init();
    clear_screen();
    printf_color(COLOR_CYAN, "RISC-V Minimal Kernel v3.0\n");
    printf("==========================================\n");

    /* 初始化物理内存管理器 */
    pmm_init();
    
    /* 测试物理内存分配 */
    test_physical_memory();

    /* 创建并激活内核页表 */
    printf("\n=== Enabling Virtual Memory ===\n");
    kvminit();
    kvminithart();
    
    printf("\n");
    printf_color(COLOR_GREEN, "Paging enabled successfully!\n");
    printf("Now running on virtual addresses.\n");

    /* 启用中断与时钟 */
    trap_init();
    printf("Timer interrupt initialized. ticks=%lu\n", (unsigned long)ticks);

    printf("System ready. Entering idle loop...\n");
    
    /* 主循环 */
    while (1) {
        static uint64 last = 0;
        if (ticks - last >= 10) {
            last = ticks;
            printf("[timer] ticks=%lu, mtime=%lu\n", (unsigned long)ticks, (unsigned long)get_time());
        }
        __asm__ volatile ("wfi");
    }
}