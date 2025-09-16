#include "uart.h"
#include "printf.h"

/* 基本功能测试 */
void test_printf_basic(void) {
    printf("=== Basic Printf Tests ===\n");
    printf("Testing integer: %d\n", 42);
    printf("Testing negative: %d\n", -123);
    printf("Testing zero: %d\n", 0);
    printf("Testing hex: 0x%x\n", 0xABC);
    printf("Testing string: %s\n", "Hello World");
    printf("Testing char: %c\n", 'X');
    printf("Testing percent: %%\n");
    printf("Testing pointer: %p\n", (void*)0x80000000);
    printf("\n");
}

/* 边界情况测试 */
void test_printf_edge_cases(void) {
    printf("=== Edge Case Tests ===\n");
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: %s\n", "");
    printf("Hex uppercase: 0X%X\n", 0xDEADBEEF);
    printf("Multiple formats: %d, %x, %s\n", 255, 255, "test");
    printf("\n");
}

/* 颜色输出测试 */
void test_color_output(void) {
    printf("=== Color Output Tests ===\n");
    printf_color(COLOR_RED, "Red text\n");
    printf_color(COLOR_GREEN, "Green text\n");
    printf_color(COLOR_BLUE, "Blue text\n");
    printf_color(COLOR_YELLOW, "Yellow text\n");
    printf("Normal text\n");
    printf("\n");
}

/* 清屏和光标测试 */
void test_screen_control(void) {
    printf("=== Screen Control Tests ===\n");
    printf("This text will be cleared in 2 seconds...\n");
    
    /* 简单延时 */
    for (volatile int i = 0; i < 10000000; i++);
    
    clear_screen();
    printf("Screen cleared!\n");
    
    printf("Testing cursor positioning:\n");
    goto_xy(10, 5);
    printf("Text at (10,5)");
    goto_xy(0, 7);
    printf("Back to normal position\n");
    
    printf("Testing line clear: ");
    printf("This will be cleared");
    clear_line();
    printf("Line cleared!\n");
    printf("\n");
}

/* 性能测试 */
void test_performance(void) {
    printf("=== Performance Test ===\n");
    printf("Outputting 100 lines...\n");
    
    for (int i = 0; i < 100; i++) {
        printf("Line %d: The quick brown fox jumps over the lazy dog\n", i);
    }
    
    printf("Performance test completed.\n\n");
}

void main(void) {
    /* 初始化UART */
    uart_init();
    
    /* 清屏并显示启动信息 */
    clear_screen();
    printf_color(COLOR_CYAN, "RISC-V Minimal Kernel v2.0\n");
    printf_color(COLOR_GREEN, "Printf and Console System Initialized\n");
    printf("==========================================\n\n");
    
    /* 运行测试 */
    test_printf_basic();
    test_printf_edge_cases();
    test_color_output();
    test_screen_control();
    
    /* 可选：性能测试（输出较多，可以注释掉） */
    // test_performance();
    
    printf_color(COLOR_YELLOW, "All tests completed successfully!\n");
    printf("System ready. Entering idle loop...\n");
    
    /* 主循环 */
    while (1) {
        __asm__ volatile ("wfi");
    }
}