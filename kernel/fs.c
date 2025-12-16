#include "fs.h"
#include "pmm.h"
#include "printf.h"
#include <stdint.h>

#define FS_MAX_FILES 16
#define FS_NAME_LEN  32
#define FS_PAGE_SIZE 4096
#define FS_MAX_FD_PER_PROC 16  // 最大文件描述符数（全局共享）

struct fs_file {
    int used;
    char name[FS_NAME_LEN];
    void *data;   /* page allocated by pmm */
    int size;
    int refcount;  /* 引用计数，支持多个文件描述符指向同一文件 */
};

// 文件描述符表项
struct fd_entry {
    int used;
    int file_idx;  /* 指向files数组的索引 */
    int offset;    /* 文件位置指针 */
};

// 文件描述符表（进一步简化：不区分进程，所有进程共享一个全局FD表）
static struct fd_entry fd_table[FS_MAX_FD_PER_PROC];

static struct fs_file files[FS_MAX_FILES];

/* 新增：跟踪通过 fs 分配的页数，便于调试输出 */
static int fs_alloc_pages = 0;

void fs_init(void){
    for (int i = 0; i < FS_MAX_FILES; i++) {
        files[i].used = 0;
        files[i].data = 0;
        files[i].size = 0;
        files[i].name[0] = 0;
        files[i].refcount = 0;  // 新增
    }
    // 初始化文件描述符表
    for (int f = 0; f < FS_MAX_FD_PER_PROC; f++) {
        fd_table[f].used = 0;
        fd_table[f].file_idx = -1;
        fd_table[f].offset = 0;
    }
    fs_alloc_pages = 0;
    printf("fs: simple in-memory fs initialized.\n");
}

/* 新增：打印当前 fs 状态 */
void fs_print_info(void){
    int used = 0;
    printf("fs: summary: alloc_pages=%d\n", fs_alloc_pages);
    printf("fs: files:\n");
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used) {
            used++;
            printf("  slot=%d name=\"%s\" size=%d data=%p\n", i, files[i].name, files[i].size, files[i].data);
        }
    }
    if (used == 0) printf("  (no files)\n");
}

static int find_slot(void){
    for (int i = 0; i < FS_MAX_FILES; i++) if (!files[i].used) return i;
    return -1;
}

static int find_by_name(const char *name){
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used) {
            int j=0; for (; j<FS_NAME_LEN && name[j] && files[i].name[j]; j++) if (files[i].name[j]!=name[j]) break;
            if (j==FS_NAME_LEN || (name[j]==0 && files[i].name[j]==0)) return i;
        }
    }
    return -1;
}

int fs_create(const char *name){
    if (!name) return -1;
    if (find_by_name(name) >= 0) return -1; /* already exists */
    int s = find_slot();
    if (s < 0) return -1;
    files[s].data = alloc_page();
    if (!files[s].data) return -1;
    fs_alloc_pages++; /* 统计 */
    files[s].used = 1;
    files[s].size = 0;
    /* copy name (simple) */
    for (int i=0;i<FS_NAME_LEN;i++){ char c = name[i]; files[s].name[i]=c; if (!c) break; }
    return s;
}

int fs_write(int fid, const void *buf, int len){
    if (fid < 0 || fid >= FS_MAX_FILES) return -1;
    if (!files[fid].used) return -1;
    if (!buf) return -1;
    if (len < 0) return -1;
    if (len > FS_PAGE_SIZE) len = FS_PAGE_SIZE;
    /* copy */
    char *dst = (char*)files[fid].data;
    const char *src = (const char*)buf;
    for (int i=0;i<len;i++) dst[i]=src[i];
    files[fid].size = len;
    return len;
}

int fs_read(int fid, void *buf, int len){
    if (fid < 0 || fid >= FS_MAX_FILES) return -1;
    if (!files[fid].used) return -1;
    if (!buf) return -1;
    if (len < 0) return -1;
    if (len > files[fid].size) len = files[fid].size;
    char *dst = (char*)buf;
    char *src = (char*)files[fid].data;
    for (int i=0;i<len;i++) dst[i]=src[i];
    return len;
}

int fs_unlink(const char *name){
    int idx = find_by_name(name);
    if (idx < 0) return -1;
    free_page(files[idx].data);
    fs_alloc_pages--; /* 更新统计 */
    files[idx].data = 0;
    files[idx].used = 0;
    files[idx].size = 0;
    files[idx].name[0] = 0;
    return 0;
}

/* 打开文件，返回文件描述符 */
int fs_open(const char *name, int flags) {
    if (!name) return -1;
    
    int file_idx = find_by_name(name);
    if (file_idx < 0) {
        // 文件不存在，如果flags包含O_CREATE则创建
        if (flags & 0x200) {  // O_CREATE标志（简化）
            file_idx = fs_create(name);
            if (file_idx < 0) return -1;
        } else {
            return -1;  // 文件不存在且不创建
        }
    }
    
    // 查找空闲的文件描述符
    for (int fd = 0; fd < FS_MAX_FD_PER_PROC; fd++) {
        if (!fd_table[fd].used) {
            fd_table[fd].used = 1;
            fd_table[fd].file_idx = file_idx;
            fd_table[fd].offset = 0;  // 从文件开头开始
            files[file_idx].refcount++;  // 增加引用计数
            return fd;
        }
    }
    return -1;  // 文件描述符用尽
}

/* 关闭文件描述符 */
int fs_close(int fd) {
    if (fd < 0 || fd >= FS_MAX_FD_PER_PROC) return -1;
    if (!fd_table[fd].used) return -1;
    
    int file_idx = fd_table[fd].file_idx;
    fd_table[fd].used = 0;
    fd_table[fd].file_idx = -1;
    fd_table[fd].offset = 0;
    
    if (file_idx >= 0 && file_idx < FS_MAX_FILES) {
        files[file_idx].refcount--;
        // 如果引用计数为0，可以考虑释放文件（但当前实现不自动释放）
    }
    
    return 0;
}

/* 改进的read：使用文件描述符和位置指针 */
int fs_read_fd(int fd, void *buf, int len) {
    if (fd < 0 || fd >= FS_MAX_FD_PER_PROC) return -1;
    if (!fd_table[fd].used) return -1;
    
    int file_idx = fd_table[fd].file_idx;
    int offset = fd_table[fd].offset;
    
    if (file_idx < 0 || file_idx >= FS_MAX_FILES) return -1;
    if (!files[file_idx].used) return -1;
    if (!buf) return -1;
    if (len < 0) return -1;
    
    // 计算可读长度
    int available = files[file_idx].size - offset;
    if (available <= 0) return 0;  // 已到文件末尾
    if (len > available) len = available;
    
    // 从offset位置读取
    char *dst = (char*)buf;
    char *src = (char*)files[file_idx].data;
    for (int i = 0; i < len; i++) {
        dst[i] = src[offset + i];
    }
    
    // 更新位置指针
    fd_table[fd].offset += len;
    
    return len;
}

/* 改进的write：使用文件描述符和位置指针 */
int fs_write_fd(int fd, const void *buf, int len) {
    if (fd < 0 || fd >= FS_MAX_FD_PER_PROC) return -1;
    if (!fd_table[fd].used) return -1;
    
    int file_idx = fd_table[fd].file_idx;
    int offset = fd_table[fd].offset;
    
    if (file_idx < 0 || file_idx >= FS_MAX_FILES) return -1;
    if (!files[file_idx].used) return -1;
    if (!buf) return -1;
    if (len < 0) return -1;
    
    // 限制写入长度
    if (offset + len > FS_PAGE_SIZE) {
        len = FS_PAGE_SIZE - offset;
    }
    if (len <= 0) return 0;
    
    // 写入到offset位置
    char *dst = (char*)files[file_idx].data;
    const char *src = (const char*)buf;
    for (int i = 0; i < len; i++) {
        dst[offset + i] = src[i];
    }
    
    // 更新文件大小和位置指针
    if (offset + len > files[file_idx].size) {
        files[file_idx].size = offset + len;
    }
    fd_table[fd].offset += len;
    
    return len;
}