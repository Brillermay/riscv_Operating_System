// filepath: /home/bri/Desktop/riscv-os/riscv_Operating_System/kernel/trap.c
#include "trap.h"
#include "riscv.h"
#include "printf.h"
#include "uart.h"

volatile uint64 ticks = 0;

// extern void supervisorvec(void); // 已移除：不再使用 S-mode 向量入口

static void timer_set_next(uint64 interval){
    uint64 hart = r_mhartid();
    uint64 now = clint_read64(CLINT_MTIME);
    clint_write64(CLINT_MTIMECMP(hart), now + interval);
}

static void timer_interrupt(void){
    // 递增节拍，设置下次触发
    ticks++;
    timer_set_next(TICK_INTERVAL);
    // 此处可挂接调度器，例如：schedule()，此实验保留占位
}

void kerneltrap(void){
    uint64 mcause = r_mcause();
    // 最高位=1表示中断
    if (mcause >> 63){
        uint64 code = mcause & 0xfff; // 原因码
        if (code == 7){ // 机器定时器中断
            timer_interrupt();
            return;
        }
        // 其他中断类型（外部、软件）可扩展
        printf("Unhandled interrupt: code=%lu\n", (unsigned long)code);
        return;
    }else{
        // 异常处理（最小实现：报告并停止）
        uint64 mepc = r_mepc();
        uint64 mtval = r_mtval();
        printf("Exception: mcause=%lx mepc=%lx mtval=%lx\n",
               (unsigned long)mcause, (unsigned long)mepc, (unsigned long)mtval);
        // 若是ecall等需前进mepc，这里可按需+4
        // w_mepc(mepc + 4);
        // 为安全起见，停机
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