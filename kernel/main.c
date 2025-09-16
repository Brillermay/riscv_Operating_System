#include "uart.h"

void main(void) {
    /* 初始化UART */
    uart_init();
    
    /* 输出Hello OS */
    uart_puts("Hello OS\n");
    uart_puts("RISC-V Minimal Kernel Started!\n");
    uart_puts("System initialized successfully.\n");
    
    /* 主循环 - 防止程序退出 */
    while (1) {
        /* 可以在这里加入更多功能 */
        __asm__ volatile ("wfi");  /* 等待中断，节省功耗 */
    }
}