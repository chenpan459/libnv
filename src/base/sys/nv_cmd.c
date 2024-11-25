
#include "nv_cmd.h"

// 封装 system 函数执行命令
int nv_system(const char* command) {
    int ret = system(command);
    if (ret == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(ret);
}

// 示例：使用封装的 nv_system 函数执行 mkfs 和 fsck 命令
int nv_mkfs(const char* filesystem_type, const char* device) {
    char command[256];
    snprintf(command, sizeof(command), "mkfs -t %s %s", filesystem_type, device);
    return nv_system(command);
}

int nv_fsck(const char* device) {
    char command[256];
    snprintf(command, sizeof(command), "fsck -y %s", device);
    return nv_system(command);
}

int nv_cmd_main() {
    const char* device = "/dev/sdb1";
    const char* filesystem_type = "ext4";

    // 创建文件系统
    if (nv_mkfs(filesystem_type, device) != 0) {
        fprintf(stderr, "Failed to create file system on %s\n", device);
        return 1;
    }
    printf("File system created on %s\n", device);

    // 检查并修复文件系统
    if (nv_fsck(device) != 0) {
        fprintf(stderr, "Failed to check file system on %s\n", device);
        return 1;
    }
    printf("File system checked on %s\n", device);

    return 0;
}