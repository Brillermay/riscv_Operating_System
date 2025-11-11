#ifndef PROC_H
#define PROC_H

#include <stdint.h>
#include "riscv.h"

/* 保证有 uint64 类型（避免重复定义冲突） */
#ifndef PROC_UINT64_DEFINED
#define PROC_UINT64_DEFINED
typedef unsigned long long uint64;
#endif

/* 最大进程数 */
#define NPROC 16

/* 进程状态 */
enum procstate { UNUSED, USED, RUNNABLE, RUNNING, SLEEPING, ZOMBIE };

/* 上下文：swtch 保存和恢复的寄存器 */
struct context {
    uint64 ra;
    uint64 sp;
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

/* 进程结构（简化）*/
struct proc {
    int pid;
    enum procstate state;
    void *kstack;             /* kernel stack bottom */
    struct context context;   /* 上下文，用于 swtch */
    void (*entry)(void);      /* 进程入口函数（内核线程） */
    void *chan;               /* 等待通道，用于 sleep/wakeup */
    int killed;
    int xstate;               /* exit status */
};

extern struct proc proc[NPROC];

/* swtch 汇编函数原型 */
extern void swtch(struct context *old, struct context *new);

/* API */
void scheduler(void);
int create_process(void (*entry)(void));
void exit_process(int status) __attribute__((noreturn));
int wait_process(int *status);

/* 进程内部调用 */
struct proc* myproc(void);
void yield(void);
void sleep(void *chan);
void wakeup(void *chan);

#endif