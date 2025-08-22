# libnv 内存池模块文档

## 概述

内存池模块是 libnv 的核心组件之一，提供了高效的内存管理功能。通过预分配内存块和智能的内存分配策略，内存池可以显著提高内存分配和释放的性能，减少内存碎片，并提供更好的内存使用统计。

## 特性

### 🚀 **高性能**
- 预分配内存块，减少系统调用
- 智能的内存分配策略
- 支持内存对齐分配
- 批量内存操作优化

### 🔧 **灵活配置**
- 可配置的初始大小和最大大小
- 支持自动扩展
- 可选的内存清零功能
- 可配置的最小块大小

### 📊 **详细统计**
- 内存使用情况统计
- 分配和释放次数统计
- 内存碎片化统计
- 实时性能监控

### 🛡️ **安全可靠**
- 内存边界检查
- 内存池完整性验证
- 错误处理和恢复
- 内存泄漏检测

## 架构设计

### 内存池结构

```
内存池 (nv_pool_t)
├── 普通内存块链表 (blocks)
│   ├── 内存块1 (nv_pool_block_t)
│   ├── 内存块2 (nv_pool_block_t)
│   └── ...
├── 大内存块链表 (large)
│   ├── 大内存块1
│   ├── 大内存块2
│   └── ...
├── 当前内存块指针 (current)
├── 配置参数
└── 统计信息
```

### 内存块结构

```
内存块 (nv_pool_block_t)
├── 下一个块指针 (next)
├── 块大小 (size)
├── 已使用大小 (used)
├── 起始地址 (start)
├── 结束地址 (end)
└── 当前可用地址 (current)
```

## API 参考

### 内存池创建和销毁

#### `nv_pool_create()`
```c
nv_pool_t* nv_pool_create(const nv_pool_config_t *config);
```
创建新的内存池实例。

**参数：**
- `config`: 内存池配置结构，如果为 NULL 则使用默认配置

**返回值：**
- 成功：返回内存池指针
- 失败：返回 NULL

#### `nv_pool_create_default()`
```c
nv_pool_t* nv_pool_create_default(void);
```
使用默认配置创建内存池。

**返回值：**
- 成功：返回内存池指针
- 失败：返回 NULL

#### `nv_pool_destroy()`
```c
void nv_pool_destroy(nv_pool_t *pool);
```
销毁内存池并释放所有相关内存。

**参数：**
- `pool`: 要销毁的内存池指针

### 内存分配函数

#### `nv_pool_alloc()`
```c
void* nv_pool_alloc(nv_pool_t *pool, size_t size);
```
从内存池分配指定大小的内存。

**参数：**
- `pool`: 内存池指针
- `size`: 要分配的内存大小（字节）

**返回值：**
- 成功：返回分配的内存指针
- 失败：返回 NULL

#### `nv_pool_calloc()`
```c
void* nv_pool_calloc(nv_pool_t *pool, size_t nmemb, size_t size);
```
分配并清零内存，类似于标准库的 calloc。

**参数：**
- `pool`: 内存池指针
- `nmemb`: 元素数量
- `size`: 每个元素的大小

**返回值：**
- 成功：返回分配的内存指针
- 失败：返回 NULL

#### `nv_pool_realloc()`
```c
void* nv_pool_realloc(nv_pool_t *pool, void *ptr, size_t size);
```
重新分配内存大小。

**参数：**
- `pool`: 内存池指针
- `ptr`: 原内存指针
- `size`: 新的大小

**返回值：**
- 成功：返回新的内存指针
- 失败：返回 NULL

#### `nv_pool_align()`
```c
void* nv_pool_align(nv_pool_t *pool, size_t size, size_t alignment);
```
分配对齐的内存。

**参数：**
- `pool`: 内存池指针
- `size`: 要分配的内存大小
- `alignment`: 对齐字节数

**返回值：**
- 成功：返回对齐的内存指针
- 失败：返回 NULL

### 内存释放函数

#### `nv_pool_free()`
```c
int nv_pool_free(nv_pool_t *pool, void *ptr);
```
释放内存池中分配的内存。

**参数：**
- `pool`: 内存池指针
- `ptr`: 要释放的内存指针

**返回值：**
- 成功：返回 NV_OK
- 失败：返回 NV_ERROR

#### `nv_pool_reset()`
```c
void nv_pool_reset(nv_pool_t *pool);
```
重置内存池，释放所有已分配的内存。

**参数：**
- `pool`: 内存池指针

### 内存池管理函数

#### `nv_pool_extend()`
```c
int nv_pool_extend(nv_pool_t *pool, size_t size);
```
手动扩展内存池。

**参数：**
- `pool`: 内存池指针
- `size`: 要扩展的大小

**返回值：**
- 成功：返回 NV_OK
- 失败：返回 NV_ERROR

#### `nv_pool_shrink()`
```c
int nv_pool_shrink(nv_pool_t *pool, size_t size);
```
收缩内存池。

**参数：**
- `pool`: 内存池指针
- `size`: 要收缩的大小

**返回值：**
- 成功：返回 NV_OK
- 失败：返回 NV_ERROR

#### `nv_pool_compact()`
```c
int nv_pool_compact(nv_pool_t *pool);
```
压缩内存池，减少内存碎片。

**参数：**
- `pool`: 内存池指针

**返回值：**
- 成功：返回 NV_OK
- 失败：返回 NV_ERROR

### 查询函数

#### `nv_pool_get_size()`
```c
size_t nv_pool_get_size(nv_pool_t *pool);
```
获取内存池总大小。

#### `nv_pool_get_used()`
```c
size_t nv_pool_get_used(nv_pool_t *pool);
```
获取已使用的内存大小。

#### `nv_pool_get_free()`
```c
size_t nv_pool_get_free(nv_pool_t *pool);
```
获取空闲内存大小。

#### `nv_pool_is_empty()`
```c
int nv_pool_is_empty(nv_pool_t *pool);
```
检查内存池是否为空。

#### `nv_pool_is_full()`
```c
int nv_pool_is_full(nv_pool_t *pool);
```
检查内存池是否已满。

### 统计信息函数

#### `nv_pool_get_stats()`
```c
void nv_pool_get_stats(nv_pool_t *pool, nv_pool_stats_t *stats);
```
获取内存池统计信息。

#### `nv_pool_reset_stats()`
```c
void nv_pool_reset_stats(nv_pool_t *pool);
```
重置统计信息。

#### `nv_pool_print_stats()`
```c
void nv_pool_print_stats(nv_pool_t *pool);
```
打印内存池统计信息。

### 调试函数

#### `nv_pool_dump()`
```c
void nv_pool_dump(nv_pool_t *pool);
```
转储内存池详细信息。

#### `nv_pool_validate()`
```c
void nv_pool_validate(nv_pool_t *pool);
```
验证内存池完整性。

## 配置选项

### 内存池配置结构

```c
typedef struct nv_pool_config_s {
    size_t initial_size;          /* 初始内存池大小 */
    size_t max_size;              /* 最大内存池大小 */
    size_t min_size;              /* 最小内存块大小 */
    int auto_extend;              /* 是否自动扩展 */
    int zero_clear;               /* 是否清零内存 */
    int enable_stats;             /* 是否启用统计 */
} nv_pool_config_t;
```

### 默认配置

```c
#define NV_POOL_CONFIG_DEFAULT { \
    .initial_size = 16 * 1024, \
    .max_size = 1024 * 1024, \
    .min_size = 4 * 1024, \
    .auto_extend = 1, \
    .zero_clear = 0, \
    .enable_stats = 1 \
}
```

## 使用示例

### 基本使用

```c
#include <nv_pool.h>

int main() {
    // 创建默认内存池
    nv_pool_t *pool = nv_pool_create_default();
    if (!pool) {
        return -1;
    }
    
    // 分配内存
    char *str = nv_pool_alloc(pool, 100);
    if (str) {
        strcpy(str, "Hello, Memory Pool!");
        printf("%s\n", str);
    }
    
    // 分配数组
    int *numbers = nv_pool_alloc_array(pool, int, 100);
    if (numbers) {
        for (int i = 0; i < 100; i++) {
            numbers[i] = i;
        }
    }
    
    // 打印统计信息
    nv_pool_print_stats(pool);
    
    // 清理
    nv_pool_destroy(pool);
    return 0;
}
```

### 自定义配置

```c
#include <nv_pool.h>

int main() {
    // 自定义配置
    nv_pool_config_t config = {
        .initial_size = 64 * 1024,     // 64KB
        .max_size = 10 * 1024 * 1024,  // 10MB
        .min_size = 8 * 1024,          // 8KB
        .auto_extend = 1,              // 启用自动扩展
        .zero_clear = 1,               // 启用内存清零
        .enable_stats = 1              // 启用统计
    };
    
    nv_pool_t *pool = nv_pool_create(&config);
    if (!pool) {
        return -1;
    }
    
    // 使用内存池...
    
    nv_pool_destroy(pool);
    return 0;
}
```

### 内存对齐分配

```c
#include <nv_pool.h>

int main() {
    nv_pool_t *pool = nv_pool_create_default();
    if (!pool) {
        return -1;
    }
    
    // 分配64字节对齐的内存
    void *aligned_ptr = nv_pool_align(pool, 1024, 64);
    if (aligned_ptr) {
        printf("对齐地址: 0x%lx\n", (unsigned long)aligned_ptr);
    }
    
    nv_pool_destroy(pool);
    return 0;
}
```

## 性能优化建议

### 1. 合理配置内存池大小
- 根据应用的内存使用模式设置初始大小
- 避免设置过大的最大大小，防止内存浪费

### 2. 使用内存池宏
```c
// 使用类型安全的分配宏
test_data_t *data = nv_pool_alloc_type(pool, test_data_t);

// 使用数组分配宏
int *array = nv_pool_alloc_array(pool, int, 1000);
```

### 3. 批量操作
- 尽量批量分配和释放内存
- 使用内存池重置功能而不是逐个释放

### 4. 监控统计信息
- 定期检查内存使用情况
- 根据统计信息调整配置参数

## 错误处理

### 常见错误

1. **内存分配失败**
   - 检查内存池是否已满
   - 检查请求的大小是否超过最大限制

2. **内存释放失败**
   - 检查指针是否有效
   - 检查指针是否来自正确的内存池

3. **内存池创建失败**
   - 检查系统内存是否充足
   - 检查配置参数是否合理

### 错误处理示例

```c
nv_pool_t *pool = nv_pool_create_default();
if (!pool) {
    nv_log_error("内存池创建失败");
    return NV_ERROR;
}

void *ptr = nv_pool_alloc(pool, size);
if (!ptr) {
    nv_log_error("内存分配失败，大小: %zu", size);
    nv_pool_destroy(pool);
    return NV_ENOMEM;
}

// 使用内存...

if (nv_pool_free(pool, ptr) != NV_OK) {
    nv_log_error("内存释放失败");
    // 处理错误...
}
```

## 线程安全

当前版本的内存池模块**不是线程安全的**。如果在多线程环境中使用，需要：

1. 为每个线程创建独立的内存池
2. 使用外部锁保护内存池操作
3. 或者等待后续版本添加线程安全支持

## 扩展性

内存池模块设计为可扩展的：

1. **插件化架构**：可以注册自定义的内存分配策略
2. **配置驱动**：支持运行时配置更改
3. **统计接口**：可以集成外部监控系统
4. **调试接口**：支持自定义调试和验证逻辑

## 未来计划

### 短期目标
- 添加线程安全支持
- 实现更智能的内存碎片整理
- 添加内存泄漏检测

### 长期目标
- 支持多种内存分配策略
- 集成硬件内存管理单元
- 支持分布式内存池

## 总结

内存池模块为 libnv 提供了高效、灵活的内存管理解决方案。通过合理使用内存池，可以显著提高应用程序的性能和稳定性。建议在开发过程中根据实际需求选择合适的配置参数，并充分利用提供的统计和调试功能来优化内存使用。
