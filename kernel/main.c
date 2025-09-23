#include "printf.h"
#include "pmm.h"
#include "vmm.h"
#include "uart.h"
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
    
    /* 启用分页后，所有代码都在虚拟地址上运行 */
    printf("\n");
    printf_color(COLOR_GREEN, "Paging enabled successfully!\n");
    printf("Now running on virtual addresses.\n");
    printf("System ready. Entering idle loop...\n");
    
    /* 主循环 */
    while (1) {
        __asm__ volatile ("wfi");
    }
}