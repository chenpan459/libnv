#include "nv_file.h"
#include <stdio.h>
#include <stdlib.h>

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
    int result = fclose(file);
    if (result != 0) {
        perror("无法关闭文件");
    }
    return result;
}


