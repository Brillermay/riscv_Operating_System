#ifndef FS_H
#define FS_H

#include <stdint.h>
#include "riscv.h"

/* 文件系统标志 */
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

/* 文件名最大长度（供 fs.c 和 syscall.c 等模块使用） */
#define FS_NAME_LEN  32

/* 极简内存文件系统接口 */
void fs_init(void);
int  fs_create(const char *name);
int  fs_write(int fid, const void *buf, int len);
int  fs_read(int fid, void *buf, int len);
int  fs_unlink(const char *name);

/* 文件描述符接口 */
int  fs_open(const char *name, int flags);
int  fs_close(int fd);
int  fs_read_fd(int fd, void *buf, int len);
int  fs_write_fd(int fd, const void *buf, int len);

/* 调试/信息打印 */
void fs_print_info(void);

#endif