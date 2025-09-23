# # 工具链前缀
# PREFIX = riscv64-unknown-elf-
# CC = $(PREFIX)gcc
# LD = $(PREFIX)ld
# OBJDUMP = $(PREFIX)objdump
# OBJCOPY = $(PREFIX)objcopy

# # 编译选项
# CFLAGS = -Wall -Werror -O2 -fno-omit-frame-pointer -ggdb
# CFLAGS += -MD -mcmodel=medany -ffreestanding -fno-common -nostdlib
# CFLAGS += -mno-relax -fno-stack-protector -fno-pie -no-pie
# CFLAGS += -Iinclude

# # 汇编选项
# ASFLAGS = -Iinclude

# # 链接选项
# LDFLAGS = -z max-page-size=4096

# # 源文件
# OBJS = kernel/entry.o kernel/main.o kernel/uart.o kernel/printf.o kernel/console.o

# # 目标文件
# TARGET = kernel.elf

# # 默认目标
# all: $(TARGET)

# # 编译规则
# %.o: %.c
# 	 $(CC) $(CFLAGS) -c $< -o $@

# %.o: %.S
# 	$(CC) $(ASFLAGS) -c $< -o $@

# # 链接
# $(TARGET): $(OBJS) kernel/kernel.ld
# 	$(LD) $(LDFLAGS) -T kernel/kernel.ld -o $@ $(OBJS)

# # 反汇编（调试用）
# dump: $(TARGET)
# 	$(OBJDUMP) -d $(TARGET) > kernel.dump

# # 查看段信息
# info: $(TARGET)
# 	$(OBJDUMP) -h $(TARGET)
# 	readelf -l $(TARGET)

# # 清理
# clean:
# 	rm -f kernel/*.o kernel/*.d $(TARGET) kernel.dump

# # QEMU运行
# qemu: $(TARGET)
# 	qemu-system-riscv64 -machine virt -bios none -kernel $(TARGET) -nographic

# # QEMU调试
# qemu-gdb: $(TARGET)
# 	qemu-system-riscv64 -machine virt -bios none -kernel $(TARGET) -nographic -s -S

# # 帮助
# help:
# 	@echo "Available targets:"
# 	@echo "  all      - Build kernel"
# 	@echo "  qemu     - Run kernel in QEMU"
# 	@echo "  qemu-gdb - Run kernel with GDB support"
# 	@echo "  dump     - Generate disassembly"
# 	@echo "  info     - Show ELF sections"
# 	@echo "  clean    - Clean build files"	
# .PHONY: all clean qemu qemu-gdb dump info help


# 工具链前缀
PREFIX = riscv64-unknown-elf-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJDUMP = $(PREFIX)objdump
OBJCOPY = $(PREFIX)objcopy

# 编译选项
CFLAGS = -Wall -Werror -O2 -fno-omit-frame-pointer -ggdb
CFLAGS += -MD -mcmodel=medany -ffreestanding -fno-common -nostdlib
CFLAGS += -mno-relax -fno-stack-protector -fno-pie -no-pie
CFLAGS += -Iinclude

# 汇编选项
ASFLAGS = -Iinclude

# 链接选项
LDFLAGS = -z max-page-size=4096

# 源文件
OBJS = kernel/entry.o kernel/main.o kernel/uart.o kernel/printf.o kernel/console.o kernel/kalloc.o kernel/vm.o

# 目标文件
TARGET = kernel.elf

# 默认目标
all: $(TARGET)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

# 链接
$(TARGET): $(OBJS) kernel/kernel.ld
	$(LD) $(LDFLAGS) -T kernel/kernel.ld -o $@ $(OBJS)

# 反汇编（调试用）
dump: $(TARGET)
	$(OBJDUMP) -d $(TARGET) > kernel.dump

# 查看段信息
info: $(TARGET)
	$(OBJDUMP) -h $(TARGET)
	readelf -l $(TARGET)

# 清理
clean:
	rm -f kernel/*.o kernel/*.d $(TARGET) kernel.dump

# QEMU运行
qemu: $(TARGET)
	qemu-system-riscv64 -machine virt -bios none -kernel $(TARGET) -nographic

# QEMU调试
qemu-gdb: $(TARGET)
	qemu-system-riscv64 -machine virt -bios none -kernel $(TARGET) -nographic -s -S

# 帮助
help:
	@echo "Available targets:"
	@echo "  all      - Build kernel"
	@echo "  qemu     - Run kernel in QEMU"
	@echo "  qemu-gdb - Run kernel with GDB support"
	@echo "  dump     - Generate disassembly"
	@echo "  info     - Show ELF sections"
	@echo "  clean    - Clean build files"

.PHONY: all clean qemu qemu-gdb dump info help