#ifndef TRAP_H
#define TRAP_H

// 简单内置类型，避免依赖libc
typedef unsigned long long uint64;
typedef unsigned int       uint32;

// 时钟频率与节拍周期（QEMU virt: mtime约10MHz，1_000_000约0.1秒）
#define TICK_INTERVAL 1000000ULL

// 对外接口
void trap_init(void);     // 设置mtvec、初始化mtimecmp、开启M态定时器中断与全局中断
uint64 get_time(void);    // 读取mtime
extern volatile uint64 ticks; // 节拍计数

#endif