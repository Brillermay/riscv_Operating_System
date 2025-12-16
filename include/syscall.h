#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* 使用项目统一的 uint64 类型（在 trap.h 中定义） */
#include "trap.h"

/* 系统调用号 */
#define SYS_getpid  1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_kill    4
#define SYS_write   5
#define SYS_read    6
#define SYS_fork    7
#define SYS_open    8   // 新增：打开文件
#define SYS_close   9   // 新增：关闭文件

/* 由 trap 调用：saved 指向 kernelvec.S 保存的寄存器区 */
void handle_syscall(uint64 *saved);

#endif