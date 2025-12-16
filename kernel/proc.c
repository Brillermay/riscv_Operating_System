#include "riscv.h"
#include "printf.h"
#include "pmm.h"
#include "proc.h"
#include "trap.h"   /* for get_time() if needed */

struct proc proc[NPROC];

static int nextpid = 1;
static struct proc *curproc = 0;
static struct context scheduler_context;

/* 内部：启动新进程的 trampoline（在进程上下文中运行）*/
static void proc_trampoline(void) {
    struct proc *p = myproc();
    if (p && p->entry) {
        p->entry();
    }
    /* 入口函数返回视为 exit */
    exit_process(0);
}

/* 返回当前进程指针（简单全局实现） */
struct proc* myproc(void) {
    return curproc;
}

/* 分配空闲进程结构（导出供fork使用） */
struct proc* allocproc(void) {
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state == UNUSED) {
            struct proc *p = &proc[i];
            p->state = USED;
            p->pid = nextpid++;
            p->kstack = alloc_page();
            if (!p->kstack) {
                p->state = UNUSED;
                return 0;
            }
            /* 清栈并设置初始上下文 */
            char *sp = (char*)p->kstack;
            for (int j = 0; j < PGSIZE; j++) sp[j] = 0;
            /* 栈顶 */
            uint64 kstack_top = (uint64)p->kstack + PGSIZE;
            for (int k = 0; k < sizeof(struct context)/8; k++) {
                ((uint64*)&p->context)[k] = 0;
            }
            p->context.sp = kstack_top;
            p->context.ra = (uint64)proc_trampoline;
            p->entry = 0;
            p->chan = 0;
            p->killed = 0;
            p->xstate = 0;
            p->parent = 0;
            p->fork_ret = -1;  /* 初始化为-1，表示未fork */
            return p;
        }
    }
    return 0;
}

/* 创建内核线程风格进程 */
int create_process(void (*entry)(void)) {
    struct proc *p = allocproc();
    if (!p) return -1;
    p->entry = entry;
    p->state = RUNNABLE;
    return p->pid;
}

/* 释放进程资源（假设已为 ZOMBIE） */
static void freeproc(struct proc *p) {
    if (p->kstack) {
        free_page(p->kstack);
        p->kstack = 0;
    }
    p->state = UNUSED;
    p->pid = 0;
    p->entry = 0;
    p->chan = 0;
    p->killed = 0;
    p->parent = 0;
    p->fork_ret = -1;
}

/* 退出当前进程（不会返回） */
void exit_process(int status) {
    struct proc *p = myproc();
    if (!p) {
        /* 没有在进程上下文中，直接停机 */
        printf("exit_process called outside process\n");
        for(;;) __asm__ volatile("wfi");
    }
    p->xstate = status;
    p->state = ZOMBIE;
    /* 切换回调度器 */
    yield();
    /* 不会返回 */
    for(;;) { __asm__ volatile("wfi"); }
}

/* 等待子进程（只等待自己的子进程） */
int wait_process(int *status) {
    struct proc *p = myproc();
    if (!p) return -1;
    
    for (;;) {
        for (int i = 0; i < NPROC; i++) {
            struct proc *child = &proc[i];
            /* 只等待自己的子进程 */
            if (child->state == ZOMBIE && child->parent == p->pid) {
                int pid = child->pid;
                if (status) *status = child->xstate;
                freeproc(child);
                return pid;
            }
        }
        /* 没找到，主动放弃CPU并重试 */
        yield();
    }
}

/* 让出CPU，回到调度器 */
void yield(void) {
    struct proc *p = myproc();
    if (!p) return;
    p->state = RUNNABLE;
    curproc = 0;
    swtch(&p->context, &scheduler_context);
}

/* sleep/wakeup（基于 chan 指针） */
void sleep(void *chan) {
    struct proc *p = myproc();
    if (!p) return;
    p->chan = chan;
    p->state = SLEEPING;
    curproc = 0;
    swtch(&p->context, &scheduler_context);
    /* 返回后，进程已经被唤醒或杀死 */
}

void wakeup(void *chan) {
    for (int i = 0; i < NPROC; i++) {
        struct proc *p = &proc[i];
        if (p->state == SLEEPING && p->chan == chan) {
            p->chan = 0;
            p->state = RUNNABLE;
        }
    }
}

/* 简单调度器：轮转调度 */
void scheduler(void) {
    printf("scheduler: starting\n");
    for (;;) {
        for (int i = 0; i < NPROC; i++) {
            struct proc *p = &proc[i];
            if (p->state != RUNNABLE) continue;
            
            // 检查进程是否被标记为killed
            if (p->killed) {
                printf("scheduler: process %d was killed\n", p->pid);
                p->state = ZOMBIE;
                p->xstate = -1;  // 被kill的进程退出码为-1
                freeproc(p);
                continue;
            }
            
            curproc = p;
            p->state = RUNNING;
            /* 切换到进程上下文 */
            swtch(&scheduler_context, &p->context);
            
            // 切换回来后再次检查killed标志
            if (p->killed && p->state != ZOMBIE) {
                printf("scheduler: process %d killed during execution\n", p->pid);
                p->state = ZOMBIE;
                p->xstate = -1;
            }
            
            /* 返回后检查是否为 ZOMBIE 并回收 */
            if (p->state == ZOMBIE) {
                freeproc(p);
            }
            curproc = 0;
        }
        /* 若没有 RUNNABLE 进程，稍作等待 */
        __asm__ volatile("wfi");
    }
}