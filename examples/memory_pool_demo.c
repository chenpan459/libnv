/************************************************
 * @文件名: memory_pool_demo.c
 * @功能: 内存池模块使用示例，展示内存池的各种功能
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，实现内存池使用示例
 ***********************************************/

#include <nv.h>
#include <nv_pool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 测试数据结构 */
typedef struct test_data_s {
    int id;
    char name[64];
    double value;
    void *buffer;
    size_t buffer_size;
} test_data_t;

/* 内存池统计回调函数 */
static int pool_stats_callback(nv_pool_block_t *block, void *data) {
    static int block_count = 0;
    
    printf("  内存块[%d]: 地址=%p, 大小=%zu, 已用=%zu, 当前=%p\n", 
           block_count++, block->start, block->size, 
           block->used, block->current);
    
    return NV_OK;
}

/* 基本内存池测试 */
static void test_basic_pool(void) {
    printf("\n=== 基本内存池测试 ===\n");
    
    /* 创建默认内存池 */
    nv_pool_t *pool = nv_pool_create_default();
    if (!pool) {
        printf("创建内存池失败\n");
        return;
    }
    
    printf("内存池创建成功\n");
    
    /* 分配一些内存 */
    char *str1 = nv_pool_alloc(pool, 100);
    char *str2 = nv_pool_alloc(pool, 200);
    int *numbers = nv_pool_alloc_array(pool, int, 100);
    
    if (str1 && str2 && numbers) {
        printf("内存分配成功\n");
        
        /* 使用内存 */
        strcpy(str1, "Hello, Memory Pool!");
        strcpy(str2, "This is a test string for memory pool demonstration.");
        for (int i = 0; i < 100; i++) {
            numbers[i] = i * 2;
        }
        
        printf("字符串1: %s\n", str1);
        printf("字符串2: %s\n", str2);
        printf("数字数组前5个: %d, %d, %d, %d, %d\n", 
               numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
    }
    
    /* 打印统计信息 */
    nv_pool_print_stats(pool);
    
    /* 清理内存池 */
    nv_pool_destroy(pool);
    printf("基本内存池测试完成\n");
}

/* 自定义配置内存池测试 */
static void test_custom_pool(void) {
    printf("\n=== 自定义配置内存池测试 ===\n");
    
    /* 创建自定义配置的内存池 */
    nv_pool_config_t config = {
        .initial_size = 32 * 1024,    /* 32KB 初始大小 */
        .max_size = 2 * 1024 * 1024,  /* 2MB 最大大小 */
        .min_size = 8 * 1024,         /* 8KB 最小块大小 */
        .auto_extend = 1,             /* 启用自动扩展 */
        .zero_clear = 1,              /* 启用内存清零 */
        .enable_stats = 1             /* 启用统计 */
    };
    
    nv_pool_t *pool = nv_pool_create(&config);
    if (!pool) {
        printf("创建自定义内存池失败\n");
        return;
    }
    
    printf("自定义内存池创建成功\n");
    
    /* 分配不同类型的内存 */
    test_data_t *data1 = nv_pool_alloc_type(pool, test_data_t);
    test_data_t *data2 = nv_pool_alloc_type(pool, test_data_t);
    
    if (data1 && data2) {
        printf("测试数据结构分配成功\n");
        
        /* 初始化数据 */
        data1->id = 1;
        strcpy(data1->name, "Test Data 1");
        data1->value = 3.14159;
        data1->buffer_size = 1024;
        data1->buffer = nv_pool_alloc(pool, data1->buffer_size);
        
        data2->id = 2;
        strcpy(data2->name, "Test Data 2");
        data2->value = 2.71828;
        data2->buffer_size = 2048;
        data2->buffer = nv_pool_alloc(pool, data2->buffer_size);
        
        /* 填充缓冲区 */
        if (data1->buffer) {
            memset(data1->buffer, 'A', data1->buffer_size);
        }
        if (data2->buffer) {
            memset(data2->buffer, 'B', data2->buffer_size);
        }
        
        printf("数据1: ID=%d, 名称=%s, 值=%.5f, 缓冲区大小=%zu\n", 
               data1->id, data1->name, data1->value, data1->buffer_size);
        printf("数据2: ID=%d, 名称=%s, 值=%.5f, 缓冲区大小=%zu\n", 
               data2->id, data2->name, data2->value, data2->buffer_size);
    }
    
    /* 测试内存对齐 */
    void *aligned_ptr = nv_pool_align(pool, 1024, 64);
    if (aligned_ptr) {
        printf("64字节对齐内存分配成功: %p\n", aligned_ptr);
        printf("对齐地址: 0x%lx\n", (unsigned long)aligned_ptr);
    }
    
    /* 打印详细统计信息 */
    nv_pool_stats_t stats;
    nv_pool_get_stats(pool, &stats);
    printf("\n内存池详细统计:\n");
    printf("  总内存块数: %zu\n", stats.total_blocks);
    printf("  总内存大小: %zu bytes (%.2f KB)\n", 
           stats.total_size, stats.total_size / 1024.0);
    printf("  已使用内存: %zu bytes (%.2f KB)\n", 
           stats.used_size, stats.used_size / 1024.0);
    printf("  空闲内存: %zu bytes (%.2f KB)\n", 
           stats.free_size, stats.free_size / 1024.0);
    printf("  分配次数: %zu\n", stats.allocation_count);
    
    /* 转储内存池信息 */
    nv_pool_dump(pool);
    
    /* 验证内存池 */
    nv_pool_validate(pool);
    
    /* 清理内存池 */
    nv_pool_destroy(pool);
    printf("自定义内存池测试完成\n");
}

/* 内存池性能测试 */
static void test_pool_performance(void) {
    printf("\n=== 内存池性能测试 ===\n");
    
    const int iterations = 100000;
    const int max_size = 1024;
    
    /* 创建性能测试内存池 */
    nv_pool_t *pool = nv_pool_create_default();
    if (!pool) {
        printf("创建性能测试内存池失败\n");
        return;
    }
    
    /* 测试内存池分配性能 */
    clock_t start = clock();
    void **ptrs = malloc(iterations * sizeof(void*));
    
    for (int i = 0; i < iterations; i++) {
        int size = (rand() % max_size) + 1;
        ptrs[i] = nv_pool_alloc(pool, size);
        if (!ptrs[i]) {
            printf("内存分配失败在迭代 %d\n", i);
            break;
        }
    }
    
    clock_t end = clock();
    double pool_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("内存池分配 %d 次耗时: %.6f 秒\n", iterations, pool_time);
    printf("平均每次分配耗时: %.9f 秒\n", pool_time / iterations);
    
    /* 测试标准malloc性能 */
    start = clock();
    void **malloc_ptrs = malloc(iterations * sizeof(void*));
    
    for (int i = 0; i < iterations; i++) {
        int size = (rand() % max_size) + 1;
        malloc_ptrs[i] = malloc(size);
        if (!malloc_ptrs[i]) {
            printf("malloc失败在迭代 %d\n", i);
            break;
        }
    }
    
    end = clock();
    double malloc_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("标准malloc分配 %d 次耗时: %.6f 秒\n", iterations, malloc_time);
    printf("平均每次malloc耗时: %.9f 秒\n", malloc_time / iterations);
    
    /* 性能比较 */
    if (pool_time < malloc_time) {
        printf("内存池比标准malloc快 %.2f%%\n", 
               ((malloc_time - pool_time) / malloc_time) * 100);
    } else {
        printf("标准malloc比内存池快 %.2f%%\n", 
               ((pool_time - malloc_time) / pool_time) * 100);
    }
    
    /* 清理内存 */
    for (int i = 0; i < iterations; i++) {
        if (ptrs[i]) {
            nv_pool_free(pool, ptrs[i]);
        }
        if (malloc_ptrs[i]) {
            free(malloc_ptrs[i]);
        }
    }
    
    free(ptrs);
    free(malloc_ptrs);
    
    /* 打印最终统计信息 */
    nv_pool_print_stats(pool);
    
    /* 清理内存池 */
    nv_pool_destroy(pool);
    printf("内存池性能测试完成\n");
}

/* 内存池扩展测试 */
static void test_pool_extension(void) {
    printf("\n=== 内存池扩展测试 ===\n");
    
    /* 创建小内存池 */
    nv_pool_config_t config = {
        .initial_size = 1024,         /* 1KB 初始大小 */
        .max_size = 64 * 1024,        /* 64KB 最大大小 */
        .min_size = 512,              /* 512B 最小块大小 */
        .auto_extend = 1,             /* 启用自动扩展 */
        .zero_clear = 0,              /* 禁用内存清零 */
        .enable_stats = 1             /* 启用统计 */
    };
    
    nv_pool_t *pool = nv_pool_create(&config);
    if (!pool) {
        printf("创建扩展测试内存池失败\n");
        return;
    }
    
    printf("扩展测试内存池创建成功\n");
    printf("初始大小: %zu bytes\n", nv_pool_get_size(pool));
    
    /* 分配内存直到触发扩展 */
    int allocation_count = 0;
    void **ptrs = malloc(1000 * sizeof(void*));
    
    while (allocation_count < 1000) {
        ptrs[allocation_count] = nv_pool_alloc(pool, 256);
        if (!ptrs[allocation_count]) {
            break;
        }
        allocation_count++;
        
        /* 每分配10次打印一次状态 */
        if (allocation_count % 10 == 0) {
            printf("已分配 %d 次, 池大小: %zu, 已用: %zu, 空闲: %zu\n", 
                   allocation_count, nv_pool_get_size(pool), 
                   nv_pool_get_used(pool), nv_pool_get_free(pool));
        }
    }
    
    printf("最终分配次数: %d\n", allocation_count);
    printf("最终池大小: %zu bytes\n", nv_pool_get_size(pool));
    printf("最终已用内存: %zu bytes\n", nv_pool_get_used(pool));
    printf("最终空闲内存: %zu bytes\n", nv_pool_get_free(pool));
    
    /* 手动扩展内存池 */
    printf("\n手动扩展内存池...\n");
    if (nv_pool_extend(pool, 32 * 1024) == NV_OK) {
        printf("手动扩展成功\n");
        printf("扩展后池大小: %zu bytes\n", nv_pool_get_size(pool));
    }
    
    /* 重置内存池 */
    printf("\n重置内存池...\n");
    nv_pool_reset(pool);
    printf("重置后池大小: %zu bytes\n", nv_pool_get_size(pool));
    printf("重置后已用内存: %zu bytes\n", nv_pool_get_used(pool));
    
    /* 清理内存 */
    for (int i = 0; i < allocation_count; i++) {
        if (ptrs[i]) {
            nv_pool_free(pool, ptrs[i]);
        }
    }
    free(ptrs);
    
    /* 清理内存池 */
    nv_pool_destroy(pool);
    printf("内存池扩展测试完成\n");
}

/* 主函数 */
int main(int argc, char *argv[]) {
    printf("libnv 内存池模块演示程序\n");
    printf("========================\n");
    
    /* 初始化随机数种子 */
    srand((unsigned int)time(NULL));
    
    /* 运行各种测试 */
    test_basic_pool();
    test_custom_pool();
    test_pool_performance();
    test_pool_extension();
    
    printf("\n所有测试完成！\n");
    return 0;
}
