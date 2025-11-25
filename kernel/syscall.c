#include "riscv.h"
#include "printf.h"
#include "proc.h"
#include "syscall.h"
#include "memlayout.h"   /* 新增：用于 KERNBASE 判断 */

extern struct proc proc[];

/* helper: write 到控制台（fd 1/2） */
static long do_write(long fd, const char *buf, long cnt) {
    if (cnt <= 0) return 0;
    if (buf == 0) return -1;

    /* 最小安全检查：若 buf 看起来是用户地址（未映射到内核空间），
       且当前未实现 copyin，则返回错误以避免触发访问异常。 */
    if ((uint64)buf < KERNBASE) {
        return -1;
    }

    if (fd != 1 && fd != 2) return -1;
    for (long i = 0; i < cnt; i++) {
        console_putc(buf[i]);
    }
    return cnt;
}

/* kill: 标记进程 killed（非常简单的实现） */
static long do_kill(int pid) {
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state != UNUSED && proc[i].pid == pid) {
            proc[i].killed = 1;
            /* 若实现了 wakeup，可在此唤醒父进程等 */
            /* wakeup(&proc[i]); */
            return 0;
        }
    }
    return -1;
}

/* 从保存区读取参数并分发
   saved 指向 kernelvec.S 保存寄存器的区域（按字64位）
*/
void handle_syscall(uint64 *saved) {
    /* kernelvec.S 保存寄存器顺序与偏移（字节）：
       a0 @ offset 64, a1 @ 72, a2 @ 80, ..., a7 @ 120
       64/8 = index 8, a7 index = 15
    */
    uint64 a0 = saved[8];
    uint64 a1 = saved[9];
    uint64 a2 = saved[10];
    uint64 syscallnum = saved[15];

    long ret = -1;
    switch (syscallnum) {
        case SYS_getpid:
            ret = myproc() ? myproc()->pid : -1;
            break;
        case SYS_exit:
            /* a0 = exit code, 不会返回 */
            exit_process((int)a0);
            ret = 0;
            break;
        case SYS_wait:
            ret = wait_process((int*)a0);
            break;
        case SYS_kill:
            ret = do_kill((int)a0);
            break;
        case SYS_write:
            ret = do_write((int)a0, (const char*)a1, (long)a2);
            break;
        default:
            printf("Unknown syscall num=%lu\n", (unsigned long)syscallnum);
            ret = -1;
            break;
    }

    /* 将返回值写回 saved a0 */
    saved[8] = (uint64)ret;
}