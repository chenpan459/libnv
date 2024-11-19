#include "nv_permissions.h"


/*
在 Unix 和 Linux 系统中，文件权限使用 9 个字符表示，分为三组，每组 3 个字符，分别表示文件所有者（User）、
  文件所属组（Group）和其他用户（Others）的权限。每个字符可以是以下三种之一：

r：读权限（Read）
w：写权限（Write）
x：执行权限（Execute）
这些权限在代码中通常使用宏定义来表示，如 S_IRUSR、S_IWUSR 等。以下是这些宏定义的含义：

S_IRUSR：文件所有者的读权限（Read permission for the owner）。对应的八进制值是 0400。
S_IWUSR：文件所有者的写权限（Write permission for the owner）。对应的八进制值是 0200。
S_IRGRP：文件所属组的读权限（Read permission for the group）。对应的八进制值是 0040。
S_IROTH：其他用户的读权限（Read permission for others）。对应的八进制值是 0004。
这些宏定义可以组合使用，以设置文件的权限。例如，S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH 表示：

文件所有者有读和写权限（r 和 w）。
文件所属组有读权限（r）。
其他用户有读权限（r）。
对应的权限字符串表示为 rw-r--r--，对应的八进制值是 0644。
*/
// 设置文件权限
int nv_set_file_permissions(const char* filepath, mode_t mode) {
    if (chmod(filepath, mode) == -1) {
        perror("chmod");
        return -1;
    }
    return 0;
}

// 获取文件权限
int nv_get_file_permissions(const char* filepath, mode_t* mode) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("stat");
        return -1;
    }
    *mode = st.st_mode;
    return 0;
}

// 设置文件所有者和组
int nv_set_file_owner(const char* filepath, const char* owner, const char* group) {
    struct passwd* pwd = getpwnam(owner);
    if (!pwd) {
        perror("getpwnam");
        return -1;
    }

    struct group* grp = getgrnam(group);
    if (!grp) {
        perror("getgrnam");
        return -1;
    }

    if (chown(filepath, pwd->pw_uid, grp->gr_gid) == -1) {
        perror("chown");
        return -1;
    }
    return 0;
}

// 获取文件所有者和组
int nv_get_file_owner(const char* filepath, char* owner, size_t owner_size, char* group, size_t group_size) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("stat");
        return -1;
    }

    struct passwd* pwd = getpwuid(st.st_uid);
    if (!pwd) {
        perror("getpwuid");
        return -1;
    }

    struct group* grp = getgrgid(st.st_gid);
    if (!grp) {
        perror("getgrgid");
        return -1;
    }

    snprintf(owner, owner_size, "%s", pwd->pw_name);
    snprintf(group, group_size, "%s", grp->gr_name);
    return 0;
}

int nv_permissions_main() {
    const char* filepath = "example_file.txt";

    // 设置文件权限
    if (nv_set_file_permissions(filepath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0) {
        return 1;
    }

    // 获取文件权限
    mode_t mode;
    if (nv_get_file_permissions(filepath, &mode) != 0) {
        return 1;
    }
    printf("File permissions: %o\n", mode & 0777);

    // 设置文件所有者和组
    if (nv_set_file_owner(filepath, "username", "groupname") != 0) {
        return 1;
    }

    // 获取文件所有者和组
    char owner[256];
    char group[256];
    if (nv_get_file_owner(filepath, owner, sizeof(owner), group, sizeof(group)) != 0) {
        return 1;
    }
    printf("File owner: %s, group: %s\n", owner, group);

    return 0;
}