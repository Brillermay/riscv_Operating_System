#include "printf.h"
#include "proc.h"
#include "syscall.h"
#include "trap.h"   /* for ticks */

extern volatile uint64 ticks;

/* 内联实现一个简单的 do_syscall，用 ecall 触发 trap，
   把 a0,a1,a2 置入相应寄存器，a7 为 syscall 号，返回 a0 */
static long do_syscall(long num, long a0, long a1, long a2) {
    long ret;
    asm volatile(
        "mv a0, %1\n"
        "mv a1, %2\n"
        "mv a2, %3\n"
        "mv a7, %4\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(ret)
        : "r"(a0), "r"(a1), "r"(a2), "r"(num)
        : "a0","a1","a2","a7","memory"
    );
    return ret;
}

static int strlen_local(const char *s) {
    int i=0;
    if (!s) return 0;
    while (s[i]) i++;
    return i;
}

/* 演示进程 */
static void demo_task(void) {
    /* 等待短时间以保证前面实验输出完成（基于相对 ticks） */
    uint64 start = ticks;
    while (ticks < start + 1) {
        yield(); /* 让出 CPU，等待调度器再次执行 */
    }

    printf("=== syscall demo: start ===\n");

    long pid = do_syscall(SYS_getpid, 0, 0, 0);
    printf("demo: getpid returned %ld\n", pid);

    const char *msg = "Demo: Hello from SYS_write\n";
    long written = do_syscall(SYS_write, 1, (long)msg, strlen_local(msg));
    printf("demo: SYS_write returned %ld\n", written);

    long bad = do_syscall(SYS_write, 1, 0x1000000, 10);
    printf("demo: SYS_write(bad ptr) returned %ld (should be -1)\n", bad);

    do_syscall(SYS_exit, 77, 0, 0);
}

/* 在系统启动早期创建 demo 进程（entry.S 调用该函数） */
void syscall_demo_init(void) {
    int pid = create_process(demo_task);
    if (pid <= 0) {
        printf("syscall_demo_init: create_process failed\n");
    } else {
        printf("syscall_demo_init: demo pid=%d created\n", pid);
    }
}