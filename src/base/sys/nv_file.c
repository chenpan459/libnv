#include "nv_file.h"


// 打开文件
FILE* nv_open_file(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        perror("无法打开文件");
    }
    return file;
}

// 读取文件
size_t nv_read_file(void* ptr, size_t size, size_t nmemb, FILE* file) {
    size_t result = fread(ptr, size, nmemb, file);
    if (result != nmemb) {
        if (feof(file)) {
            printf("到达文件末尾\n");
        } else if (ferror(file)) {
            perror("读取文件时出错");
        }
    }
    return result;
}

// 写入文件
size_t nv_write_file(const void* ptr, size_t size, size_t nmemb, FILE* file) {
    size_t result = fwrite(ptr, size, nmemb, file);
    if (result != nmemb) {
        perror("写入文件时出错");
    }
    return result;
}

// 关闭文件
int nv_close_file(FILE* file) {
    if (fclose(file) != 0) {
        perror("关闭文件时出错");
        return EOF;
    }
    return 0;
}

// 获取文件大小
long nv_get_file_size(FILE* file) {
    long current_pos = ftell(file);
    if (current_pos == -1) {
        perror("获取当前位置时出错");
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("移动到文件末尾时出错");
        return -1;
    }

    long size = ftell(file);
    if (size == -1) {
        perror("获取文件大小时出错");
        return -1;
    }

    if (fseek(file, current_pos, SEEK_SET) != 0) {
        perror("恢复文件位置时出错");
        return -1;
    }

    return size;
}

// 移动文件指针
int nv_seek_file(FILE* file, long offset, int whence) {
    if (fseek(file, offset, whence) != 0) {
        perror("移动文件指针时出错");
        return -1;
    }
    return 0;
}

// 获取当前文件指针位置
long nv_tell_file(FILE* file) {
    long pos = ftell(file);
    if (pos == -1) {
        perror("获取文件指针位置时出错");
    }
    return pos;
}
