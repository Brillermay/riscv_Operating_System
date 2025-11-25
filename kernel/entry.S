.section .text.init
.global _start

_start:
    # 关闭中断，设置机器模式
    csrw mie, zero
    csrw mip, zero
    
    # 设置栈指针 - 栈从高地址向下增长
    la sp, stack_top
    
    # 输出启动标记 'S'
    li t0, 0x10000000    # UART基地址
    li t1, 'S'
    sb t1, 0(t0)
    
    # 清零BSS段
    la t0, bss_start
    la t1, bss_end
clear_bss:
    bgeu t0, t1, bss_done
    sb zero, 0(t0)
    addi t0, t0, 1
    j clear_bss
bss_done:
    
    # 输出栈设置完成标记 'P'
    li t0, 0x10000000
    li t1, 'P'
    sb t1, 0(t0)
    
    # 跳转到主程序
    call main

halt:
    j halt

.section .bss
.align 4
stack_bottom:
    .space 4096
stack_top: