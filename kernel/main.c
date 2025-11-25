// filepath: /home/bri/Desktop/riscv-os/riscv_Operating_System/kernel/main.c
#include "printf.h"
#include "pmm.h"
#include "vmm.h"
#include "uart.h"
#include "trap.h"
#include "proc.h"   /* 新增：process APIs */

/* 新增：demo 初始化函数原型（定义在 kernel/syscall_test.c）*/
void syscall_demo_init(void);

static void cpu_task(void) {
    for (int i = 0; i < 5; i++) {
        printf("[task %d] pid=%d loop=%d ticks=%lu\n", 1, myproc() ? myproc()->pid : -1, i, (unsigned long)ticks);
        for (volatile int j = 0; j < 1000000; j++); /* 简单忙等 */
        yield();
    }
    printf("[task] exiting pid=%d\n", myproc() ? myproc()->pid : -1);
}

/* 另一测试任务 */
static void tcp_task(void) {
    for (int i = 0; i < 3; i++) {
        printf("[tcp] pid=%d iter=%d\n", myproc() ? myproc()->pid : -1, i);
        yield();
    }
    exit_process(0);
}

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
    
#if 0
    /* 原来实验测试注释掉 */
    test_physical_memory();
#endif

    /* 创建并激活内核页表 */
    printf("\n=== Enabling Virtual Memory ===\n");
    kvminit();
    kvminithart();
    
    printf("\n");
    printf_color(COLOR_GREEN, "Paging enabled successfully!\n");
    printf("Now running on virtual addresses.\n");

    /* 启用中断与时钟（为调度器准备） */
    trap_init();
    printf("Timer interrupt initialized. ticks=%lu\n", (unsigned long)ticks);

    { 
        /* -- 新增：实验四 中断测试（在进入调度器前的空闲打印） */
        printf("System ready. Entering idle loop...\n");
        uint64 last = ticks;
        /* 打印到达到一定 ticks 为止，按你的示例打印到 250 */
        while (ticks < 50) {
            if (ticks - last >= 10) {
                last = ticks;
                printf("[timer] ticks=%lu, mtime=%lu\n",
                       (unsigned long)ticks, (unsigned long)get_time());
            }
            /* 省电等待中断 */
            __asm__ volatile("wfi");
        }
        printf("Idle timer test complete. Proceeding to process creation...\n");
    }

    { 
        /* 实验5启动标识 */
        printf_color(COLOR_YELLOW, "\nExperiment 5: Process management & scheduling - START\n");

        /* 创建测试进程 */
        int pid1 = create_process(cpu_task);
        int pid2 = create_process(tcp_task);
        printf("Created processes pid=%d pid=%d\n", pid1, pid2);

        /* 创建系统调用演示进程（创建但演示任务会等待 ticks，保证输出在之前实验之后） */
        syscall_demo_init();

        /* 进入调度器（不会返回） */
        scheduler();
    }

    /* 不会走到这里 */
    for (;;) __asm__ volatile("wfi");
}