/************************************************
 * @文件名: nv_pool.h
 * @功能: libnv内存池模块头文件，定义内存池管理接口
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义内存池管理接口
 * 2024-11-04 - 完善内存池架构设计
 ***********************************************/

#ifndef _NV_POOL_H_INCLUDED_
#define _NV_POOL_H_INCLUDED_

#include <nv_config.h>
#include <nv_core.h>

/* 内存池块大小定义 */
#define NV_POOL_MIN_SIZE       (4 * 1024)      /* 最小内存池大小 4KB */
#define NV_POOL_MAX_SIZE       (16 * 1024 * 1024) /* 最大内存池大小 16MB */
#define NV_POOL_ALIGNMENT      8               /* 内存对齐字节数 */
#define NV_POOL_ALIGNMENT_MASK (NV_POOL_ALIGNMENT - 1)

/* 内存池块结构 */
typedef struct nv_pool_block_s {
    struct nv_pool_block_s *next;  /* 下一个内存块 */
    size_t size;                   /* 内存块大小 */
    size_t used;                   /* 已使用大小 */
    char *start;                   /* 内存块起始地址 */
    char *end;                     /* 内存块结束地址 */
    char *current;                 /* 当前可用地址 */
} nv_pool_block_t;

/* 内存池统计信息 */
typedef struct nv_pool_stats_s {
    size_t total_blocks;           /* 总内存块数 */
    size_t total_size;             /* 总内存大小 */
    size_t used_size;              /* 已使用内存大小 */
    size_t free_size;              /* 空闲内存大小 */
    size_t allocation_count;       /* 分配次数 */
    size_t deallocation_count;     /* 释放次数 */
    size_t fragmentation_count;    /* 碎片化次数 */
} nv_pool_stats_t;

/* 内存池结构 */
typedef struct nv_pool_s {
    nv_pool_block_t *blocks;      /* 内存块链表 */
    nv_pool_block_t *current;     /* 当前内存块 */
    nv_pool_block_t *large;       /* 大内存块链表 */
    size_t max_size;              /* 单个内存块最大大小 */
    size_t min_size;              /* 单个内存块最小大小 */
    nv_pool_stats_t stats;        /* 统计信息 */
    int auto_extend;              /* 是否自动扩展 */
    int zero_clear;               /* 是否清零内存 */
    void *private_data;           /* 私有数据 */
} nv_pool_t;

/* 内存池配置结构 */
typedef struct nv_pool_config_s {
    size_t initial_size;          /* 初始内存池大小 */
    size_t max_size;              /* 最大内存池大小 */
    size_t min_size;              /* 最小内存块大小 */
    int auto_extend;              /* 是否自动扩展 */
    int zero_clear;               /* 是否清零内存 */
    int enable_stats;             /* 是否启用统计 */
} nv_pool_config_t;

/* 内存池配置默认值 */
#define NV_POOL_CONFIG_DEFAULT { \
    .initial_size = 16 * 1024, \
    .max_size = 1024 * 1024, \
    .min_size = 4 * 1024, \
    .auto_extend = 1, \
    .zero_clear = 0, \
    .enable_stats = 1 \
}

/* 内存池创建和销毁 */
nv_pool_t* nv_pool_create(const nv_pool_config_t *config);
nv_pool_t* nv_pool_create_default(void);
void nv_pool_destroy(nv_pool_t *pool);

/* 内存分配函数 */
void* nv_pool_alloc(nv_pool_t *pool, size_t size);
void* nv_pool_calloc(nv_pool_t *pool, size_t nmemb, size_t size);
void* nv_pool_realloc(nv_pool_t *pool, void *ptr, size_t size);
void* nv_pool_align(nv_pool_t *pool, size_t size, size_t alignment);

/* 内存释放函数 */
int nv_pool_free(nv_pool_t *pool, void *ptr);
void nv_pool_reset(nv_pool_t *pool);

/* 内存池管理函数 */
int nv_pool_extend(nv_pool_t *pool, size_t size);
int nv_pool_shrink(nv_pool_t *pool, size_t size);
int nv_pool_compact(nv_pool_t *pool);

/* 内存池查询函数 */
size_t nv_pool_get_size(nv_pool_t *pool);
size_t nv_pool_get_used(nv_pool_t *pool);
size_t nv_pool_get_free(nv_pool_t *pool);
int nv_pool_is_empty(nv_pool_t *pool);
int nv_pool_is_full(nv_pool_t *pool);

/* 统计信息函数 */
void nv_pool_get_stats(nv_pool_t *pool, nv_pool_stats_t *stats);
void nv_pool_reset_stats(nv_pool_t *pool);
void nv_pool_print_stats(nv_pool_t *pool);

/* 内存池迭代函数 */
typedef int (*nv_pool_iterate_cb)(nv_pool_block_t *block, void *data);
int nv_pool_iterate(nv_pool_t *pool, nv_pool_iterate_cb callback, void *data);

/* 内存池调试函数 */
void nv_pool_dump(nv_pool_t *pool);
void nv_pool_validate(nv_pool_t *pool);

/* 内存池工具宏 */
#define nv_pool_alloc_type(pool, type) \
    (type*)nv_pool_alloc(pool, sizeof(type))

#define nv_pool_calloc_type(pool, type, count) \
    (type*)nv_pool_calloc(pool, count, sizeof(type))

#define nv_pool_alloc_array(pool, type, count) \
    (type*)nv_pool_alloc(pool, (count) * sizeof(type))

/* 内存池创建宏 */
#define NV_POOL_CREATE(name, config) \
    nv_pool_t *name = nv_pool_create(config); \
    if (!name) { \
        return NV_ERROR; \
    }

#define NV_POOL_CREATE_DEFAULT(name) \
    nv_pool_t *name = nv_pool_create_default(); \
    if (!name) { \
        return NV_ERROR; \
    }

/* 内存池销毁宏 */
#define NV_POOL_DESTROY(name) \
    if (name) { \
        nv_pool_destroy(name); \
        name = NULL; \
    }

/* 内存池安全分配宏 */
#define NV_POOL_SAFE_ALLOC(pool, ptr, size) \
    do { \
        ptr = nv_pool_alloc(pool, size); \
        if (!ptr) { \
            return NV_ENOMEM; \
        } \
    } while(0)

#define NV_POOL_SAFE_CALLOC(pool, ptr, nmemb, size) \
    do { \
        ptr = nv_pool_calloc(pool, nmemb, size); \
        if (!ptr) { \
            return NV_ENOMEM; \
        } \
    } while(0)

#endif /* _NV_POOL_H_INCLUDED_ */
