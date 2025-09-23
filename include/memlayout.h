#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

/* QEMU virt平台的物理内存布局 */
#define KERNBASE 0x80000000L          // 内核起始虚拟地址
#define PHYSTOP  (KERNBASE + 128*1024*1024) // 物理内存顶部 (128MB)

/* 设备地址 */
#define UART0    0x10000000L

#endif