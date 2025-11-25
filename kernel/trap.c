// filepath: /home/bri/Desktop/riscv-os/riscv_Operating_System/kernel/trap.c
#include "trap.h"
#include "riscv.h"
#include "printf.h"
#include "uart.h"
#include "syscall.h"

volatile uint64 ticks = 0;

static void timer_set_next(uint64 interval){
    uint64 hart = r_mhartid();
    uint64 now = clint_read64(CLINT_MTIME);
    clint_write64(CLINT_MTIMECMP(hart), now + interval);
}

static void timer_interrupt(void){
    ticks++;
    timer_set_next(TICK_INTERVAL);
}

/* 修改：kerneltrap 接收 saved 指针（由 kernelvec.S 放在 a0） */
void kerneltrap(uint64 *saved){
    uint64 mcause = r_mcause();
    if (mcause >> 63){
        uint64 code = mcause & 0xfff;
        if (code == 7){
            timer_interrupt();
            return;
        }
        printf("Unhandled interrupt: code=%lu\n", (unsigned long)code);
        return;
    } else {
        uint64 cause = mcause & 0xfff;
        if (cause == 11) { /* ecall from M-mode */
            /* DEBUG: dump saved a0..a7 before dispatch */
            printf("[dbg] saved[8..15]:");
            for (int i = 8; i <= 15; i++) {
                printf(" %llx", (unsigned long long)saved[i]);
            }
            printf("\n");

            /* 调用系统调用分发器 */
            handle_syscall(saved);

            /* DEBUG: dump returned a0 */
            printf("[dbg] after handle, saved[8]=%llx\n", (unsigned long long)saved[8]);

            /* advance mepc to skip ecall */
            w_mepc(r_mepc() + 4);
            return;
        }

        uint64 mepc = r_mepc();
        uint64 mtval = r_mtval();
        printf("Exception: mcause=%lx mepc=%lx mtval=%lx\n",
               (unsigned long)mcause, (unsigned long)mepc, (unsigned long)mtval);
        while (1) { __asm__ volatile("wfi"); }
    }
}

uint64 get_time(void){
    return clint_read64(CLINT_MTIME);
}

void trap_init(void){
    extern void kernelvec(void);

    // 设置 M-mode 陷阱向量（保留 M-mode 入口）
    w_mtvec((uint64)kernelvec);

    // 先设置第一次定时器触发点，再开中断
    timer_set_next(TICK_INTERVAL);

    // 使能M态定时器中断
    set_mie(MIE_MTIE);
    // 开启全局M态中断
    set_mstatus(MSTATUS_MIE);
}