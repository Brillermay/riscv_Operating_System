#include "riscv.h"
#include "printf.h"
#include "trap.h"

extern volatile uint64 ticks;

void supervisortrap(void){
    uint64 scause = r_scause();
    if (scause >> 63) {
        uint64 code = scause & 0xfff;
        if (code == 5) { // Supervisor timer interrupt (STIP)
            // 更新节拍并设置下次中断
            ticks++;
            uint64 hart = r_mhartid();
            uint64 now = clint_read64(CLINT_MTIME);
            clint_write64(CLINT_MTIMECMP(hart), now + TICK_INTERVAL);
            return;
        }
        printf("Unhandled S-mode interrupt: code=%lu\n", (unsigned long)code);
        return;
    } else {
        // S-mode 异常：简单报告并停机（或回退）
        uint64 sepc = r_sepc();
        uint64 stval = r_stval();
        printf("S-mode Exception: scause=%lx sepc=%lx stval=%lx\n",
               (unsigned long)scause, (unsigned long)sepc, (unsigned long)stval);
        while (1) { __asm__ volatile("wfi"); }
    }
}