#include "printf.h"
#include "fs.h"
#include "proc.h"
#include "trap.h"
#include <stdint.h>

/* demo 任务：等待短时间保证前面实验输出完成，然后演示 fs API */
static void fs_demo_task(void){
    extern volatile uint64 ticks;
    uint64 start = ticks;
    while (ticks < start + 1) {
        yield();
    }

    printf("=== fs demo: start ===\n");

    /* 打印初始状态 */
    fs_print_info();

    int fid = fs_create("testfile");
    if (fid < 0) {
        printf("fs demo: create failed\n");
        exit_process(1);
    }
    printf("fs demo: created fid=%d\n", fid);
    fs_print_info();

    const char *msg = "Hello, filesystem!";
    int w = fs_write(fid, msg, 19);
    printf("fs demo: write returned %d\n", w);

    char buf[64];
    int r = fs_read(fid, buf, sizeof(buf));
    if (r > 0) {
        if (r < (int)sizeof(buf)) buf[r] = 0;
        else buf[sizeof(buf)-1] = 0;
        printf("fs demo: read returned %d content=\"%s\"\n", r, buf);
    } else {
        printf("fs demo: read failed\n");
    }

    if (fs_unlink("testfile") == 0) {
        printf("fs demo: unlink OK\n");
    } else {
        printf("fs demo: unlink failed\n");
    }

    /* 打印最终状态 */
    fs_print_info();

    // 测试open/close系统调用
    printf("\n=== Testing open/close ===\n");
    int fd = fs_open("testfile2", O_CREATE | O_RDWR);
    if (fd >= 0) {
        printf("fs demo: opened fd=%d\n", fd);
        const char *msg2 = "Test open/close";
        fs_write_fd(fd, msg2, 16);
        fs_close(fd);
        printf("fs demo: closed fd=%d\n", fd);
    }

    exit_process(0);
}

/* 初始化时创建 demo 进程并打印 pid，便于确认被创建 */
void fs_demo_init(void){
    int pid = create_process(fs_demo_task);
    printf("fs_demo_init: demo pid=%d created\n", pid);
}