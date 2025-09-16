#ifndef PRINTF_H
#define PRINTF_H

/* 可变参数支持 */
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

/* 格式化输出函数 */
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);

/* 控制台功能 */
void console_putc(char c);
void console_puts(const char *s);
void clear_screen(void);
void goto_xy(int x, int y);
void clear_line(void);

/* 颜色定义 */
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

void printf_color(int color, const char *fmt, ...);

#endif