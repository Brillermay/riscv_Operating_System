OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    /* QEMU virt平台起始地址 */
    . = 0x80000000;
    
    /* 代码段 */
    .text : {
        *(.text.init)   /* 启动代码放在最前面 */
        *(.text)
        *(.text.*)
    }
    etext = .; 
    
    . = ALIGN(4096);
    
    /* 只读数据段 */
    .rodata : {
        . = ALIGN(16);
        *(.rodata)
        *(.rodata.*)
    }
    
    /* 数据段 */
    .data : {
        . = ALIGN(16);
        *(.data)
        *(.data.*)
    }
    
    /* BSS段 */
    .bss : {
        . = ALIGN(16);
        bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        bss_end = .;
    }
    
    /* 定义结束符号 */
    end = .;
}