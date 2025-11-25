.section .text
.align 4
.globl kernelvec
kernelvec:
    addi sp, sp, -256

    /* 保存寄存器到栈，布局使 a0 位于 64 字节偏移（saved[8]），a7 位于 120（saved[15]） */
    sd ra,   0(sp)
    sd gp,   8(sp)
    sd tp,  16(sp)
    sd t0,  24(sp)
    sd t1,  32(sp)
    sd t2,  40(sp)
    sd s0,  48(sp)
    sd s1,  56(sp)
    sd a0,  64(sp)
    sd a1,  72(sp)
    sd a2,  80(sp)
    sd a3,  88(sp)
    sd a4,  96(sp)
    sd a5, 104(sp)
    sd a6, 112(sp)
    sd a7, 120(sp)
    sd s2, 128(sp)
    sd s3, 136(sp)
    sd s4, 144(sp)
    sd s5, 152(sp)
    sd s6, 160(sp)
    sd s7, 168(sp)
    sd s8, 176(sp)
    sd s9, 184(sp)
    sd s10,192(sp)
    sd s11,200(sp)
    sd t3, 208(sp)
    sd t4, 216(sp)
    sd t5, 224(sp)
    sd t6, 232(sp)

    /* 可选：保存 mcause/mepc 到已知偏移，供调试 */
    csrr t0, mcause
    csrr t1, mepc
    sd t0, 240(sp)
    sd t1, 248(sp)

    /* 把原始 saved 指针放到 a0 作第一个参数，然后调用 kerneltrap(saved) */
    mv a0, sp
    call kerneltrap

    /* 恢复寄存器（顺序与保存相反） */
    ld t6, 232(sp)
    ld t5, 224(sp)
    ld t4, 216(sp)
    ld t3, 208(sp)
    ld s11,200(sp)
    ld s10,192(sp)
    ld s9, 184(sp)
    ld s8, 176(sp)
    ld s7, 168(sp)
    ld s6, 160(sp)
    ld s5, 152(sp)
    ld s4, 144(sp)
    ld s3, 136(sp)
    ld s2, 128(sp)
    ld a7, 120(sp)
    ld a6, 112(sp)
    ld a5, 104(sp)
    ld a4, 96(sp)
    ld a3, 88(sp)
    ld a2, 80(sp)
    ld a1, 72(sp)
    ld a0, 64(sp)
    ld s1, 56(sp)
    ld s0, 48(sp)
    ld t2, 40(sp)
    ld t1, 32(sp)
    ld t0, 24(sp)
    ld tp, 16(sp)
    ld gp, 8(sp)
    ld ra, 0(sp)

    addi sp, sp, 256
    mret