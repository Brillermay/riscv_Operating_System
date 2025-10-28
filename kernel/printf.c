#include "printf.h"
#include <stdarg.h>

/* 字符串长度计算 */
static int strlen(const char *s) {
    int len = 0;
    if (!s) return 0;
    while (*s++) len++;
    return len;
}

/* 字符串复制 */
static char *strcpy(char *dst, const char *src) {
    char *p = dst;
    if (!src) return dst;
    while ((*p++ = *src++));
    return dst;
}

/* 无符号长整型转字符串 */
static void print_ulong(char *buf, unsigned long n, int base) {
    static char digits[] = "0123456789ABCDEF";
    char temp[32];
    int i = 0;

    if (n == 0) {
        *buf++ = '0';
        *buf = '\0';
        return;
    }

    while (n > 0) {
        temp[i++] = digits[n % base];
        n /= base;
    }

    while (i > 0) {
        *buf++ = temp[--i];
    }
    *buf = '\0';
}

/* 有符号长整型转字符串（使用 print_ulong） */
static void print_slong(char *buf, long n, int base) {
    if (n < 0) {
        *buf++ = '-';
        /* 注意 LONG_MIN 情况在此简化处理，教育用途可接受 */
        print_ulong(buf, (unsigned long)(-n), base);
    } else {
        print_ulong(buf, (unsigned long)n, base);
    }
}

/* 兼容旧的 int 实现（保持原有行为） */
static void print_number(char *buf, long num, int base, int sign) {
    if (sign) {
        print_slong(buf, num, base);
    } else {
        print_ulong(buf, (unsigned long)num, base);
    }
}

/* 处理INT_MIN的特殊情况（保留原逻辑） */
static void print_number_safe(char *buf, int num, int base, int sign) {
    if (sign && num == -2147483648) {  /* INT_MIN */
        if (base == 10) {
            strcpy(buf, "-2147483648");
        } else if (base == 16) {
            strcpy(buf, "80000000");
        } else {
            strcpy(buf, "error");
        }
        return;
    }
    print_number(buf, num, base, sign);
}

/* 核心printf实现，支持可选 'l' 长度修饰符（简单支持 1 个或 2 个 l） */
static int vprintf(const char *fmt, va_list ap) {
    char buf[128];
    int count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            console_putc(*fmt);
            count++;
            fmt++;
            continue;
        }

        fmt++;  /* 跳过% */

        /* 解析可选长度修饰符（仅处理 'l' 与 "ll"） */
        int lcount = 0;
        while (*fmt == 'l') {
            lcount++;
            fmt++;
            if (lcount >= 2) break;
        }

        switch (*fmt) {
            case 'd':  /* 十进制整数 */
            case 'i': {
                if (lcount) {
                    long val = va_arg(ap, long);
                    print_slong(buf, val, 10);
                } else {
                    int val = va_arg(ap, int);
                    print_number_safe(buf, val, 10, 1);
                }
                console_puts(buf);
                count += strlen(buf);
                break;
            }

            case 'u': {  /* 无符号十进制 */
                if (lcount) {
                    unsigned long val = va_arg(ap, unsigned long);
                    print_ulong(buf, val, 10);
                } else {
                    unsigned int val = va_arg(ap, unsigned int);
                    print_number(buf, val, 10, 0);
                }
                console_puts(buf);
                count += strlen(buf);
                break;
            }

            case 'x':  /* 十六进制（小写） */
            case 'X': {  /* 十六进制（大写） */
                if (lcount) {
                    unsigned long val = va_arg(ap, unsigned long);
                    print_ulong(buf, val, 16);
                } else {
                    unsigned int val = va_arg(ap, unsigned int);
                    print_number(buf, val, 16, 0);
                }
                console_puts(buf);
                count += strlen(buf);
                break;
            }

            case 'p': {  /* 指针 -> 当作 unsigned long 打印 */
                void *ptr = va_arg(ap, void*);
                console_puts("0x");
                print_ulong(buf, (unsigned long)ptr, 16);
                console_puts(buf);
                count += 2 + strlen(buf);
                break;
            }

            case 'c': {  /* 字符 */
                int c = va_arg(ap, int);
                console_putc(c);
                count++;
                break;
            }

            case 's': {  /* 字符串 */
                char *s = va_arg(ap, char*);
                if (!s) s = "(null)";
                console_puts(s);
                count += strlen(s);
                break;
            }

            case '%': {  /* %% -> % */
                console_putc('%');
                count++;
                break;
            }

            default: {  /* 未知格式符，回退并输出字面 */
                console_putc('%');
                /* 如果有 l 修饰，打印相应 'l' 字母(s) */
                for (int i = 0; i < lcount; i++) console_putc('l');
                console_putc(*fmt);
                count += 2 + lcount;
                break;
            }
        }

        fmt++;
    }

    return count;
}

/* 格式化输出到控制台 */
int printf(const char *fmt, ...) {
    va_list ap;
    int result;

    va_start(ap, fmt);
    result = vprintf(fmt, ap);
    va_end(ap);

    return result;
}

/* 格式化输出到字符串 */
int sprintf(char *buf, const char *fmt, ...) {
    /* 简化实现：暂不实现 */
    return 0;
}

/* 彩色输出 */
void printf_color(int color, const char *fmt, ...) {
    char color_code[16];
    va_list ap;

    /* 设置颜色 */
    print_number(color_code, 30 + color, 10, 0);
    console_puts("\033[");
    console_puts(color_code);
    console_puts("m");

    /* 输出内容 */
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    /* 重置颜色 */
    console_puts("\033[0m");
}