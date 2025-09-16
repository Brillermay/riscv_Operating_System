#include "printf.h"

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

/* 数字转字符串 - 核心算法 */
static void print_number(char *buf, long num, int base, int sign) {
    static char digits[] = "0123456789ABCDEF";
    char temp[32];
    int i = 0;
    unsigned long n;
    
    /* 处理负数 */
    if (sign && num < 0) {
        *buf++ = '-';
        n = -num;  /* 注意：这里对INT_MIN可能溢出，实际需要特殊处理 */
    } else {
        n = num;
    }
    
    /* 处理0的特殊情况 */
    if (n == 0) {
        *buf++ = '0';
        *buf = '\0';
        return;
    }
    
    /* 逆序生成数字字符 */
    while (n > 0) {
        temp[i++] = digits[n % base];
        n /= base;
    }
    
    /* 正序复制到输出缓冲区 */
    while (i > 0) {
        *buf++ = temp[--i];
    }
    *buf = '\0';
}

/* 处理INT_MIN的特殊情况 */
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

/* 核心printf实现 */
static int vprintf(const char *fmt, va_list ap) {
    char buf[64];
    int count = 0;
    
    while (*fmt) {
        if (*fmt != '%') {
            console_putc(*fmt);
            count++;
            fmt++;
            continue;
        }
        
        fmt++;  /* 跳过% */
        
        switch (*fmt) {
            case 'd':  /* 十进制整数 */
            case 'i': {
                int val = va_arg(ap, int);
                print_number_safe(buf, val, 10, 1);
                console_puts(buf);
                count += strlen(buf);
                break;
            }
            
            case 'u': {  /* 无符号十进制 */
                unsigned int val = va_arg(ap, unsigned int);
                print_number(buf, val, 10, 0);
                console_puts(buf);
                count += strlen(buf);
                break;
            }
            
            case 'x':  /* 十六进制（小写） */
            case 'X': {  /* 十六进制（大写） */
                unsigned int val = va_arg(ap, unsigned int);
                print_number(buf, val, 16, 0);
                console_puts(buf);
                count += strlen(buf);
                break;
            }
            
            case 'p': {  /* 指针 */
                void *ptr = va_arg(ap, void*);
                console_puts("0x");
                print_number(buf, (unsigned long)ptr, 16, 0);
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
            
            default: {  /* 未知格式符 */
                console_putc('%');
                console_putc(*fmt);
                count += 2;
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
    /* 简化实现：重定向输出到缓冲区 */
    /* 这里需要实现一个缓冲区版本的vprintf */
    /* 为简化，暂时返回0 */
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