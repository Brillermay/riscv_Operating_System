#include "uart.h"

/* UART 16550 寄存器定义 */
#define UART_BASE 0x10000000
#define UART_THR  (UART_BASE + 0)  /* Transmit Holding Register */
#define UART_LSR  (UART_BASE + 5)  /* Line Status Register */
#define LSR_THRE  (1 << 5)         /* Transmit Holding Register Empty */

/* 读取寄存器 */
static inline unsigned char uart_read_reg(unsigned long addr) {
    return *(volatile unsigned char*)addr;
}

/* 写入寄存器 */
static inline void uart_write_reg(unsigned long addr, unsigned char val) {
    *(volatile unsigned char*)addr = val;
}

/* 等待发送缓冲区为空 */
static void uart_wait_tx_ready(void) {
    while ((uart_read_reg(UART_LSR) & LSR_THRE) == 0) {
        /* 忙等待 */
    }
}

/* 发送一个字符 */
void uart_putc(char c) {
    uart_wait_tx_ready();
    uart_write_reg(UART_THR, c);
}

/* 发送字符串 */
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

/* UART初始化 */
void uart_init(void) {
    /* QEMU的UART已经预初始化，这里可以为空 */
    /* 或者加入更多的配置代码 */
}