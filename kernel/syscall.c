#include "riscv.h"
#include "printf.h"
#include "proc.h"
#include "syscall.h"
#include "memlayout.h"
#include "vmm.h"  // 新增：用于获取进程页表
#include "fs.h"   // 文件系统接口

extern struct proc proc[];

/* 从用户空间拷贝数据到内核空间
   注意：当前实现是简化版本，因为所有进程共享内核页表
   完整实现需要遍历用户页表验证地址有效性
*/
static int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len) {
    // 基本边界检查：简单限制在 39 位虚拟地址范围内
    if (srcva >= (1L << 39) || srcva + len > (1L << 39)) {
        return -1;  // 地址超出39位虚拟地址空间
    }

    // 目前还没有真正的“用户页表”支持，这里做一个保守的安全检查：
    // - 对于小于 KERNBASE 的地址，一律认为是“用户空间”且暂时不可信，直接报错
    //   这样可以避免像 0x01000000 这样的坏指针导致访存异常。
    if (srcva < KERNBASE) {
        return -1;
    }

    // 对于内核空间地址（>= KERNBASE），直接拷贝。
    // 这覆盖了当前 demo 中的“用户缓冲区”（实际仍在内核栈中）的情况。
    char *src = (char*)srcva;
    for (uint64 i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    return 0;
}

/* 从内核空间拷贝数据到用户空间（当前未使用，如需可启用） */
// static int copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len) {
//     if (dstva >= (1L << 39) || dstva + len > (1L << 39)) {
//         return -1;
//     }
//     
//     // 简化实现：直接拷贝
//     char *dst = (char*)dstva;
//     for (uint64 i = 0; i < len; i++) {
//         dst[i] = src[i];
//     }
//     return 0;
// }

/* 改进的write系统调用，支持用户空间地址 */
static long do_write(long fd, const char *buf, long cnt) {
    if (cnt <= 0) return 0;
    if (buf == 0) return -1;
    
    if (fd == 1 || fd == 2) {
        // 标准输出/错误，使用原来的控制台输出
        // 判断是用户地址还是内核地址
        if ((uint64)buf < KERNBASE) {
            // 用户空间地址，需要拷贝到内核缓冲区
            char kbuf[256];  // 内核缓冲区
            long total_written = 0;
            long remaining = cnt;
            
            // 分段拷贝和输出（处理大于缓冲区的情况）
            while (remaining > 0) {
                long to_copy = remaining < 256 ? remaining : 256;
                
                // 从用户空间拷贝到内核缓冲区
                if (copyin(0, kbuf, (uint64)buf + total_written, to_copy) < 0) {
                    return total_written > 0 ? total_written : -1;
                }
                
                // 输出到控制台
                for (long i = 0; i < to_copy; i++) {
                    console_putc(kbuf[i]);
                }
                
                total_written += to_copy;
                remaining -= to_copy;
            }
            return total_written;
        } else {
            // 内核地址，直接使用
            for (long i = 0; i < cnt; i++) {
                console_putc(buf[i]);
            }
            return cnt;
        }
    } else if (fd >= 0) {
        // 文件描述符，使用文件系统
        char kbuf[256];
        long total_written = 0;
        long remaining = cnt;
        
        while (remaining > 0) {
            long to_copy = remaining < 256 ? remaining : 256;
            
            // 从用户空间拷贝到内核缓冲区
            if (copyin(0, kbuf, (uint64)buf + total_written, to_copy) < 0) {
                return total_written > 0 ? total_written : -1;
            }
            
            // 写入文件
            long n = fs_write_fd(fd, kbuf, to_copy);
            if (n <= 0) break;
            
            total_written += n;
            remaining -= n;
            if (n < to_copy) break;
        }
        
        return total_written;
    } else {
        return -1;
    }
}

/* 改进的kill系统调用 */
static long do_kill(int pid) {
    struct proc *p;
    
    // 不能杀死自己
    if (myproc() && myproc()->pid == pid) {
        return -1;
    }
    
    // 查找目标进程
    for (int i = 0; i < NPROC; i++) {
        p = &proc[i];
        if (p->state != UNUSED && p->pid == pid) {
            p->killed = 1;
            
            // 如果进程在睡眠，唤醒它以便检查killed标志
            if (p->state == SLEEPING) {
                p->state = RUNNABLE;
                wakeup(p->chan);
            }
            
            return 0;
        }
    }
    return -1;  // 进程不存在
}

/* fork系统调用：创建当前进程的副本
   返回值：父进程返回子进程pid，子进程返回0
   注意：这是内核线程风格的简化fork，不复制页表
*/
static long do_fork(void) {
    struct proc *p = myproc();
    if (!p) {
        printf("fork: no current process\n");
        return -1;
    }
    
    // 分配新进程结构
    struct proc *np = allocproc();
    if (!np) {
        printf("fork: no free process slot\n");
        return -1;
    }
    
    // 复制父进程的上下文
    np->context = p->context;
    
    // 确保子进程有独立的内核栈
    if (np->kstack) {
        // 设置子进程的栈指针（新栈）
        uint64 kstack_top = (uint64)np->kstack + PGSIZE;
        np->context.sp = kstack_top;
    }
    
    // 设置父子关系
    np->parent = p->pid;
    
    // 关键：设置fork返回值
    // 父进程直接返回子进程pid（通过函数返回值）
    // 子进程通过fork_ret返回0（在handle_syscall中处理）
    np->fork_ret = 0;        // 子进程返回0
    
    // 子进程状态设为RUNNABLE
    np->state = RUNNABLE;
    
    // 父进程返回子进程pid
    return np->pid;
}

// 在do_fork函数后添加文件系统系统调用处理函数

/* open系统调用 */
static long do_open(const char *pathname, int flags) {
    if (!pathname) return -1;
    
    // 从用户空间拷贝路径名（简化：假设是内核地址）
    char kpath[FS_NAME_LEN];
    int pathlen = 0;
    while (pathlen < FS_NAME_LEN - 1 && pathname[pathlen]) {
        kpath[pathlen] = pathname[pathlen];
        pathlen++;
    }
    kpath[pathlen] = 0;
    
    return fs_open(kpath, flags);
}

/* close系统调用 */
static long do_close(int fd) {
    return fs_close(fd);
}

/* read系统调用（文件系统版本） */
static long do_read(int fd, void *buf, long count) {
    if (fd < 0 || !buf || count < 0) return -1;
    
    // 判断是标准输入还是文件描述符
    if (fd == 0) {
        // 标准输入（当前不支持）
        return -1;
    } else if (fd == 1 || fd == 2) {
        // 标准输出/错误（不应该read）
        return -1;
    } else {
        // 文件描述符
        char kbuf[256];
        long total_read = 0;
        long remaining = count;
        
        while (remaining > 0) {
            long to_read = remaining < 256 ? remaining : 256;
            long n = fs_read_fd(fd, kbuf, to_read);
            if (n <= 0) break;
            
            // 拷贝到用户空间（简化：直接拷贝）
            char *ubuf = (char*)buf;
            for (long i = 0; i < n; i++) {
                ubuf[total_read + i] = kbuf[i];
            }
            
            total_read += n;
            remaining -= n;
            if (n < to_read) break;  // 到达文件末尾
        }
        
        return total_read;
    }
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

    // 验证系统调用号范围
    if (syscallnum > 10) {  // 假设最大系统调用号为10
        printf("Invalid syscall number: %lu\n", (unsigned long)syscallnum);
        saved[8] = -1;
        return;
    }

    long ret = -1;
    switch (syscallnum) {
        case SYS_getpid:
            ret = myproc() ? myproc()->pid : -1;
            break;
        case SYS_exit:
            /* a0 = exit code, 不会返回 */
            exit_process((int)a0);
            ret = 0;  // 不会执行到这里
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
        case SYS_fork:
            ret = do_fork();
            // fork的特殊处理：检查fork_ret字段
            // 如果设置了fork_ret，使用它作为返回值
            if (myproc() && myproc()->fork_ret >= 0) {
                ret = myproc()->fork_ret;
                myproc()->fork_ret = -1;  // 清除标志，只使用一次
            }
            break;
        case SYS_open:
            ret = do_open((const char*)a0, (int)a1);
            break;
        case SYS_close:
            ret = do_close((int)a0);
            break;
        case SYS_read:
            ret = do_read((int)a0, (void*)a1, (long)a2);
            break;
        default:
            printf("Unknown syscall num=%lu\n", (unsigned long)syscallnum);
            ret = -1;
            break;
    }

    /* 将返回值写回 saved a0 */
    saved[8] = (uint64)ret;
}