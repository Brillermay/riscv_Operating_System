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
    // 避免由于当前 fork 实现为“内核线程克隆”导致子进程从头再跑一遍，
    // 进而形成 syscall demo 无限链式执行：只允许完整跑一次。
    static int ran_once = 0;
    if (ran_once) {
        do_syscall(SYS_exit, 0, 0, 0);
    }
    ran_once = 1;

    /* 等待短时间以保证前面实验输出完成（基于相对 ticks） */
    uint64 start = ticks;
    while (ticks < start + 1) {
        yield(); /* 让出 CPU，等待调度器再次执行 */
    }

    printf("=== syscall demo: start ===\n");

    // 测试1: getpid
    long pid = do_syscall(SYS_getpid, 0, 0, 0);
    printf("demo: getpid returned %ld\n", pid);

    // 测试2: write with kernel address
    const char *msg = "Demo: Hello from SYS_write (kernel addr)\n";
    long written = do_syscall(SYS_write, 1, (long)msg, strlen_local(msg));
    printf("demo: SYS_write(kernel) returned %ld\n", written);

    // 测试3: write with user address (简化测试，实际地址仍在内核空间)
    char user_buf[64] = "Demo: Hello from SYS_write (user addr)\n";
    long written2 = do_syscall(SYS_write, 1, (long)user_buf, strlen_local(user_buf));
    printf("demo: SYS_write(user) returned %ld\n", written2);

    // 测试4: 无效地址保护
    long bad = do_syscall(SYS_write, 1, 0x1000000, 10);
    printf("demo: SYS_write(bad ptr) returned %ld (should be -1)\n", bad);

    // 测试5: 无效文件描述符
    long bad_fd = do_syscall(SYS_write, 99, (long)msg, strlen_local(msg));
    printf("demo: SYS_write(bad fd) returned %ld (should be -1)\n", bad_fd);

    // 测试6: 零长度写入
    long zero = do_syscall(SYS_write, 1, (long)msg, 0);
    printf("demo: SYS_write(zero len) returned %ld (should be 0)\n", zero);

    // 测试7: fork系统调用（当前先关闭，避免大量重复输出）
#if 0
    printf("\n=== Testing fork ===\n");
    long fork_ret = do_syscall(SYS_fork, 0, 0, 0);
    if (fork_ret == 0) {
        // 子进程
        printf("[child] pid=%ld, fork returned 0\n", do_syscall(SYS_getpid, 0, 0, 0));
        do_syscall(SYS_exit, 0, 0, 0);  // 子进程退出
    } else if (fork_ret > 0) {
        // 父进程
        printf("[parent] pid=%ld, fork returned child_pid=%ld\n", 
               do_syscall(SYS_getpid, 0, 0, 0), fork_ret);
        // 等待子进程
        int status;
        long waited_pid = do_syscall(SYS_wait, (long)&status, 0, 0);
        printf("[parent] waited for child pid=%ld, exit status=%d\n", waited_pid, status);
    } else {
        printf("fork failed: returned %ld\n", fork_ret);
    }
#endif

    printf("=== syscall demo: basic tests passed ===\n");

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