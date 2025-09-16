#include "printf.h"
#include "uart.h"

/* 控制台状态 */
static int console_x = 0;  /* 当前光标X位置 */
static int console_y = 0;  /* 当前光标Y位置 */

#define CONSOLE_WIDTH  80
#define CONSOLE_HEIGHT 25

/* 输出单个字符到控制台 */
void console_putc(char c) {
    switch (c) {
        case '\n':
            console_x = 0;
            console_y++;
            if (console_y >= CONSOLE_HEIGHT) {
                console_y = CONSOLE_HEIGHT - 1;
                /* 这里可以实现滚屏，现在简化处理 */
            }
            uart_putc('\r');  /* 回车 */
            uart_putc('\n');  /* 换行 */
            break;
            
        case '\r':
            console_x = 0;
            uart_putc('\r');
            break;
            
        case '\t':
            /* Tab处理：对齐到8的倍数 */
            do {
                console_putc(' ');
            } while (console_x % 8 != 0);
            break;
            
        case '\b':
            /* 退格处理 */
            if (console_x > 0) {
                console_x--;
                uart_putc('\b');
                uart_putc(' ');
                uart_putc('\b');
            }
            break;
            
        default:
            if (c >= 32 && c <= 126) {  /* 可打印字符 */
                console_x++;
                if (console_x >= CONSOLE_WIDTH) {
                    console_putc('\n');
                }
                uart_putc(c);
            }
            break;
    }
}

/* 输出字符串到控制台 */
void console_puts(const char *s) {
    if (!s) return;
    
    while (*s) {
        console_putc(*s);
        s++;
    }
}

/* 清屏功能 */
void clear_screen(void) {
    /* 发送ANSI转义序列清屏 */
    uart_puts("\033[2J");    /* 清除整个屏幕 */
    uart_puts("\033[H");     /* 光标回到左上角 */
    
    /* 重置控制台状态 */
    console_x = 0;
    console_y = 0;
}

/* 光标定位 */
void goto_xy(int x, int y) {
    char buf[32];
    
    /* 构造ANSI转义序列 */
    char *p = buf;
    *p++ = '\033';
    *p++ = '[';
    
    /* 转换Y坐标 */
    if (y >= 10) {
        *p++ = '0' + y / 10;
    }
    *p++ = '0' + y % 10;
    *p++ = ';';
    
    /* 转换X坐标 */
    if (x >= 10) {
        *p++ = '0' + x / 10;
    }
    *p++ = '0' + x % 10;
    *p++ = 'H';
    *p = '\0';
    
    uart_puts(buf);
    
    console_x = x;
    console_y = y;
}

/* 清除当前行 */
void clear_line(void) {
    uart_puts("\033[K");  /* 清除从光标到行尾 */
}