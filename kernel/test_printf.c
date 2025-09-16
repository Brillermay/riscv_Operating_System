#include "uart.h"

/* 基本功能测试 */
void test_printf_basic(void) {
    printf("=== Basic printf tests ===\n");
    printf("Testing integer: %d\n", 42);
    printf("Testing negative: %d\n", -123);
    printf("Testing zero: %d\n", 0);
    printf("Testing hex: 0x%x\n", 0xABC);
    printf("Testing octal: %o\n", 64);
    printf("Testing unsigned: %u\n", 4294967295U);
    printf("Testing string: %s\n", "Hello World");
    printf("Testing char: %c\n", 'X');
    printf("Testing percent: %%\n");
    printf("Testing pointer: %p\n", (void*)0x80000000);
    printf("\n");
}

/* 边界情况测试 */
void test_printf_edge_cases(void) {
    printf("=== Edge case tests ===\n");
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: %s\n", "");
    printf("Zero hex: 0x%x\n", 0);
    printf("Large hex: 0x%x\n", 0xFFFFFFFF);
    printf("Unknown format: %z\n", 123);
    printf("\n");
}

/* 性能测试 */
void test_printf_performance(void) {
    int i;
    printf("=== Performance test ===\n");
    printf("Printing 100 numbers:\n");
    
    for (i = 0; i < 100; i++) {
        printf("%d ", i);
        if ((i + 1) % 10 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

/* 屏幕控制测试 */
void test_screen_control(void) {
    printf("=== Screen control test ===\n");
    printf("This text will be cleared in 2 seconds...\n");
    
    /* 简单延时 */
    volatile int delay;
    for (delay = 0; delay < 10000000; delay++);
    
    clear_screen();
    printf("Screen cleared!\n");
    printf("Testing goto_xy...\n");
    
    goto_xy(10, 5);
    printf("This is at (10,5)");
    
    goto_xy(1, 8);
    printf("Back to normal position\n");
    
    printf("Testing clear_line: ");
    printf("This line will be partially cleared");
    goto_xy(25, 10);
    clear_line();
    printf("Cleared!\n");
}