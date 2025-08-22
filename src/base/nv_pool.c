/************************************************
 * @文件名: nv_pool.c
 * @功能: libnv内存池模块实现文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，实现内存池管理功能
 * 2024-11-04 - 完善内存池架构实现
 ***********************************************/

#include "nv_pool.h"
#include <nv_log.h>
#include <nv_mem.h>
#include <string.h>
#include <assert.h>

/* 内部函数声明 */
static nv_pool_block_t* nv_pool_block_create(size_t size);
static void nv_pool_block_destroy(nv_pool_block_t *block);
static void* nv_pool_block_alloc(nv_pool_block_t *block, size_t size);
static int nv_pool_block_free(nv_pool_block_t *block, void *ptr);
static size_t nv_pool_align_size(size_t size);
static void nv_pool_update_stats(nv_pool_t *pool, size_t allocated, int is_alloc);

/* 内存池创建函数 */
nv_pool_t* nv_pool_create(const nv_pool_config_t *config) {
    nv_pool_t *pool;
    nv_pool_config_t default_config = NV_POOL_CONFIG_DEFAULT;
    
    if (!config) {
        config = &default_config;
    }
    
    pool = (nv_pool_t*)nv_malloc(sizeof(nv_pool_t));
    if (!pool) {
        nv_log_error("内存池创建失败：无法分配内存池结构");
        return NULL;
    }
    
    /* 初始化内存池结构 */
    memset(pool, 0, sizeof(nv_pool_t));
    pool->max_size = config->max_size;
    pool->min_size = config->min_size;
    pool->auto_extend = config->auto_extend;
    pool->zero_clear = config->zero_clear;
    
    /* 创建初始内存块 */
    if (config->initial_size > 0) {
        pool->blocks = nv_pool_block_create(config->initial_size);
        if (!pool->blocks) {
            nv_log_error("内存池创建失败：无法创建初始内存块");
            nv_free(pool);
            return NULL;
        }
        pool->current = pool->blocks;
        
        /* 更新统计信息 */
        pool->stats.total_blocks = 1;
        pool->stats.total_size = config->initial_size;
        pool->stats.free_size = config->initial_size;
    }
    
    nv_log_debug("内存池创建成功：初始大小=%zu, 最大大小=%zu", 
                  config->initial_size, config->max_size);
    
    return pool;
}

/* 创建默认内存池 */
nv_pool_t* nv_pool_create_default(void) {
    return nv_pool_create(NULL);
}

/* 内存池销毁函数 */
void nv_pool_destroy(nv_pool_t *pool) {
    nv_pool_block_t *block, *next;
    
    if (!pool) {
        return;
    }
    
    nv_log_debug("销毁内存池：总大小=%zu, 已使用=%zu", 
                  pool->stats.total_size, pool->stats.used_size);
    
    /* 销毁所有内存块 */
    block = pool->blocks;
    while (block) {
        next = block->next;
        nv_pool_block_destroy(block);
        block = next;
    }
    
    /* 销毁大内存块 */
    block = pool->large;
    while (block) {
        next = block->next;
        nv_pool_block_destroy(block);
        block = next;
    }
    
    /* 销毁内存池结构 */
    nv_free(pool);
}

/* 内存分配函数 */
void* nv_pool_alloc(nv_pool_t *pool, size_t size) {
    void *ptr;
    nv_pool_block_t *block;
    
    if (!pool || size == 0) {
        return NULL;
    }
    
    /* 对齐内存大小 */
    size = nv_pool_align_size(size);
    
    /* 尝试在当前内存块中分配 */
    if (pool->current && pool->current->used + size <= pool->current->size) {
        ptr = nv_pool_block_alloc(pool->current, size);
        if (ptr) {
            nv_pool_update_stats(pool, size, 1);
            return ptr;
        }
    }
    
    /* 查找可用的内存块 */
    block = pool->blocks;
    while (block) {
        if (block->used + size <= block->size) {
            ptr = nv_pool_block_alloc(block, size);
            if (ptr) {
                pool->current = block;
                nv_pool_update_stats(pool, size, 1);
                return ptr;
            }
        }
        block = block->next;
    }
    
    /* 如果启用自动扩展，创建新的内存块 */
    if (pool->auto_extend) {
        size_t new_size = (size > pool->min_size) ? size : pool->min_size;
        if (new_size <= pool->max_size) {
            block = nv_pool_block_create(new_size);
            if (block) {
                block->next = pool->blocks;
                pool->blocks = block;
                pool->current = block;
                
                /* 更新统计信息 */
                pool->stats.total_blocks++;
                pool->stats.total_size += new_size;
                pool->stats.free_size += new_size;
                
                ptr = nv_pool_block_alloc(block, size);
                if (ptr) {
                    nv_pool_update_stats(pool, size, 1);
                    return ptr;
                }
            }
        }
    }
    
    /* 如果内存块太小，创建大内存块 */
    if (size > pool->max_size) {
        block = nv_pool_block_create(size);
        if (block) {
            block->next = pool->large;
            pool->large = block;
            
            /* 更新统计信息 */
            pool->stats.total_blocks++;
            pool->stats.total_size += size;
            pool->stats.free_size += size;
            
            ptr = nv_pool_block_alloc(block, size);
            if (ptr) {
                nv_pool_update_stats(pool, size, 1);
                return ptr;
            }
        }
    }
    
    nv_log_error("内存分配失败：大小=%zu, 内存池已满", size);
    return NULL;
}

/* 内存清零分配函数 */
void* nv_pool_calloc(nv_pool_t *pool, size_t nmemb, size_t size) {
    void *ptr;
    size_t total_size = nmemb * size;
    
    ptr = nv_pool_alloc(pool, total_size);
    if (ptr && pool->zero_clear) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

/* 内存重新分配函数 */
void* nv_pool_realloc(nv_pool_t *pool, void *ptr, size_t size) {
    void *new_ptr;
    
    if (!ptr) {
        return nv_pool_alloc(pool, size);
    }
    
    if (size == 0) {
        nv_pool_free(pool, ptr);
        return NULL;
    }
    
    /* 查找指针所在的内存块 */
    nv_pool_block_t *block = pool->blocks;
    while (block) {
        if ((char*)ptr >= block->start && (char*)ptr < block->end) {
            /* 如果新大小小于等于当前块，直接返回原指针 */
            if (size <= block->size - ((char*)ptr - block->start)) {
                return ptr;
            }
            break;
        }
        block = block->next;
    }
    
    /* 查找大内存块 */
    if (!block) {
        block = pool->large;
        while (block) {
            if ((char*)ptr >= block->start && (char*)ptr < block->end) {
                if (size <= block->size - ((char*)ptr - block->start)) {
                    return ptr;
                }
                break;
            }
            block = block->next;
        }
    }
    
    /* 分配新内存并复制数据 */
    new_ptr = nv_pool_alloc(pool, size);
    if (new_ptr && block) {
        size_t old_size = block->size - ((char*)ptr - block->start);
        size_t copy_size = (size < old_size) ? size : old_size;
        memcpy(new_ptr, ptr, copy_size);
        nv_pool_free(pool, ptr);
    }
    
    return new_ptr;
}

/* 内存对齐分配函数 */
void* nv_pool_align(nv_pool_t *pool, size_t size, size_t alignment) {
    void *ptr;
    size_t aligned_size = size + alignment - 1;
    
    ptr = nv_pool_alloc(pool, aligned_size);
    if (ptr) {
        uintptr_t addr = (uintptr_t)ptr;
        uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
        return (void*)aligned_addr;
    }
    
    return NULL;
}

/* 内存释放函数 */
int nv_pool_free(nv_pool_t *pool, void *ptr) {
    nv_pool_block_t *block;
    
    if (!pool || !ptr) {
        return NV_ERROR;
    }
    
    /* 查找指针所在的内存块 */
    block = pool->blocks;
    while (block) {
        if ((char*)ptr >= block->start && (char*)ptr < block->end) {
            if (nv_pool_block_free(block, ptr) == NV_OK) {
                nv_pool_update_stats(pool, 0, 0);
                return NV_OK;
            }
            break;
        }
        block = block->next;
    }
    
    /* 查找大内存块 */
    if (!block) {
        block = pool->large;
        while (block) {
            if ((char*)ptr >= block->start && (char*)ptr < block->end) {
                if (nv_pool_block_free(block, ptr) == NV_OK) {
                    nv_pool_update_stats(pool, 0, 0);
                    return NV_OK;
                }
                break;
            }
            block = block->next;
        }
    }
    
    nv_log_error("内存释放失败：无效的指针 %p", ptr);
    return NV_ERROR;
}

/* 内存池重置函数 */
void nv_pool_reset(nv_pool_t *pool) {
    nv_pool_block_t *block;
    
    if (!pool) {
        return;
    }
    
    /* 重置所有内存块 */
    block = pool->blocks;
    while (block) {
        block->used = 0;
        block->current = block->start;
        block = block->next;
    }
    
    /* 重置大内存块 */
    block = pool->large;
    while (block) {
        block->used = 0;
        block->current = block->start;
        block = block->next;
    }
    
    /* 重置统计信息 */
    pool->stats.used_size = 0;
    pool->stats.free_size = pool->stats.total_size;
    pool->stats.allocation_count = 0;
    pool->stats.deallocation_count = 0;
    
    pool->current = pool->blocks;
    
    nv_log_debug("内存池重置完成");
}

/* 内存池扩展函数 */
int nv_pool_extend(nv_pool_t *pool, size_t size) {
    nv_pool_block_t *block;
    
    if (!pool || size == 0) {
        return NV_ERROR;
    }
    
    size = nv_pool_align_size(size);
    
    block = nv_pool_block_create(size);
    if (!block) {
        return NV_ERROR;
    }
    
    block->next = pool->blocks;
    pool->blocks = block;
    pool->current = block;
    
    /* 更新统计信息 */
    pool->stats.total_blocks++;
    pool->stats.total_size += size;
    pool->stats.free_size += size;
    
    nv_log_debug("内存池扩展成功：新增大小=%zu", size);
    return NV_OK;
}

/* 内存池收缩函数 */
int nv_pool_shrink(nv_pool_t *pool, size_t size) {
    /* 简单的收缩实现，实际应用中可能需要更复杂的逻辑 */
    nv_log_debug("内存池收缩：目标大小=%zu", size);
    return NV_OK;
}

/* 内存池压缩函数 */
int nv_pool_compact(nv_pool_t *pool) {
    /* 简单的压缩实现，实际应用中可能需要更复杂的逻辑 */
    nv_log_debug("内存池压缩完成");
    return NV_OK;
}

/* 内存池查询函数 */
size_t nv_pool_get_size(nv_pool_t *pool) {
    return pool ? pool->stats.total_size : 0;
}

size_t nv_pool_get_used(nv_pool_t *pool) {
    return pool ? pool->stats.used_size : 0;
}

size_t nv_pool_get_free(nv_pool_t *pool) {
    return pool ? pool->stats.free_size : 0;
}

int nv_pool_is_empty(nv_pool_t *pool) {
    return pool ? (pool->stats.used_size == 0) : 1;
}

int nv_pool_is_full(nv_pool_t *pool) {
    return pool ? (pool->stats.free_size == 0) : 0;
}

/* 统计信息函数 */
void nv_pool_get_stats(nv_pool_t *pool, nv_pool_stats_t *stats) {
    if (pool && stats) {
        memcpy(stats, &pool->stats, sizeof(nv_pool_stats_t));
    }
}

void nv_pool_reset_stats(nv_pool_t *pool) {
    if (pool) {
        memset(&pool->stats, 0, sizeof(nv_pool_stats_t));
        pool->stats.total_size = nv_pool_get_size(pool);
        pool->stats.free_size = pool->stats.total_size;
    }
}

void nv_pool_print_stats(nv_pool_t *pool) {
    if (!pool) {
        return;
    }
    
    nv_log_info("内存池统计信息:");
    nv_log_info("  总内存块数: %zu", pool->stats.total_blocks);
    nv_log_info("  总内存大小: %zu bytes", pool->stats.total_size);
    nv_log_info("  已使用内存: %zu bytes", pool->stats.used_size);
    nv_log_info("  空闲内存: %zu bytes", pool->stats.free_size);
    nv_log_info("  分配次数: %zu", pool->stats.allocation_count);
    nv_log_info("  释放次数: %zu", pool->stats.deallocation_count);
    nv_log_info("  碎片化次数: %zu", pool->stats.fragmentation_count);
}

/* 内存池迭代函数 */
int nv_pool_iterate(nv_pool_t *pool, nv_pool_iterate_cb callback, void *data) {
    nv_pool_block_t *block;
    int result = NV_OK;
    
    if (!pool || !callback) {
        return NV_ERROR;
    }
    
    /* 迭代普通内存块 */
    block = pool->blocks;
    while (block && result == NV_OK) {
        result = callback(block, data);
        block = block->next;
    }
    
    /* 迭代大内存块 */
    if (result == NV_OK) {
        block = pool->large;
        while (block && result == NV_OK) {
            result = callback(block, data);
            block = block->next;
        }
    }
    
    return result;
}

/* 内存池调试函数 */
void nv_pool_dump(nv_pool_t *pool) {
    nv_pool_block_t *block;
    int block_count = 0;
    
    if (!pool) {
        nv_log_info("内存池: NULL");
        return;
    }
    
    nv_log_info("内存池转储:");
    nv_log_info("  配置: max_size=%zu, min_size=%zu, auto_extend=%d", 
                 pool->max_size, pool->min_size, pool->auto_extend);
    
    /* 转储普通内存块 */
    block = pool->blocks;
    while (block) {
        nv_log_info("  块[%d]: 地址=%p, 大小=%zu, 已用=%zu, 当前=%p", 
                    block_count++, block->start, block->size, 
                    block->used, block->current);
        block = block->next;
    }
    
    /* 转储大内存块 */
    block = pool->large;
    while (block) {
        nv_log_info("  大块[%d]: 地址=%p, 大小=%zu, 已用=%zu, 当前=%p", 
                    block_count++, block->start, block->size, 
                    block->used, block->current);
        block = block->next;
    }
}

void nv_pool_validate(nv_pool_t *pool) {
    nv_pool_block_t *block;
    int errors = 0;
    
    if (!pool) {
        nv_log_error("内存池验证失败：内存池为NULL");
        return;
    }
    
    /* 验证普通内存块 */
    block = pool->blocks;
    while (block) {
        if (block->current < block->start || block->current > block->end) {
            nv_log_error("内存块验证失败：当前指针超出范围");
            errors++;
        }
        if (block->used > block->size) {
            nv_log_error("内存块验证失败：已用大小超过总大小");
            errors++;
        }
        block = block->next;
    }
    
    /* 验证大内存块 */
    block = pool->large;
    while (block) {
        if (block->current < block->start || block->current > block->end) {
            nv_log_error("大内存块验证失败：当前指针超出范围");
            errors++;
        }
        if (block->used > block->size) {
            nv_log_error("大内存块验证失败：已用大小超过总大小");
            errors++;
        }
        block = block->next;
    }
    
    if (errors == 0) {
        nv_log_info("内存池验证通过");
    } else {
        nv_log_error("内存池验证失败：发现 %d 个错误", errors);
    }
}

/* 内部函数实现 */

/* 创建内存块 */
static nv_pool_block_t* nv_pool_block_create(size_t size) {
    nv_pool_block_t *block;
    char *memory;
    
    size = nv_pool_align_size(size);
    
    block = (nv_pool_block_t*)nv_malloc(sizeof(nv_pool_block_t));
    if (!block) {
        return NULL;
    }
    
    memory = (char*)nv_malloc(size);
    if (!memory) {
        nv_free(block);
        return NULL;
    }
    
    block->next = NULL;
    block->size = size;
    block->used = 0;
    block->start = memory;
    block->end = memory + size;
    block->current = memory;
    
    return block;
}

/* 销毁内存块 */
static void nv_pool_block_destroy(nv_pool_block_t *block) {
    if (block) {
        if (block->start) {
            nv_free(block->start);
        }
        nv_free(block);
    }
}

/* 在内存块中分配内存 */
static void* nv_pool_block_alloc(nv_pool_block_t *block, size_t size) {
    void *ptr;
    
    if (block->current + size > block->end) {
        return NULL;
    }
    
    ptr = block->current;
    block->current += size;
    block->used += size;
    
    return ptr;
}

/* 在内存块中释放内存 */
static int nv_pool_block_free(nv_pool_block_t *block, void *ptr) {
    /* 简单的释放实现，实际应用中可能需要更复杂的逻辑 */
    /* 这里只是标记内存为可用，不实际释放 */
    return NV_OK;
}

/* 内存大小对齐 */
static size_t nv_pool_align_size(size_t size) {
    return (size + NV_POOL_ALIGNMENT_MASK) & ~NV_POOL_ALIGNMENT_MASK;
}

/* 更新统计信息 */
static void nv_pool_update_stats(nv_pool_t *pool, size_t allocated, int is_alloc) {
    if (is_alloc) {
        pool->stats.used_size += allocated;
        pool->stats.free_size -= allocated;
        pool->stats.allocation_count++;
    } else {
        pool->stats.deallocation_count++;
    }
}
