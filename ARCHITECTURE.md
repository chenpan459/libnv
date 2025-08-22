# libnv 架构设计文档

## 1. 项目概述

libnv 是一个面向 Linux 平台的高性能基础库，提供网络、文件、内存、日志等基础功能。该库采用模块化设计，支持事件驱动、异步I/O等现代编程模式。

## 2. 整体架构

### 2.1 架构层次

```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (Application Layer)                │
├─────────────────────────────────────────────────────────────┤
│                    工具层 (Utility Layer)                   │
├─────────────────────────────────────────────────────────────┤
│                    系统层 (System Layer)                    │
├─────────────────────────────────────────────────────────────┤
│                    网络层 (Network Layer)                   │
├─────────────────────────────────────────────────────────────┤
│                    事件层 (Event Layer)                     │
├─────────────────────────────────────────────────────────────┤
│                    基础层 (Base Layer)                      │
├─────────────────────────────────────────────────────────────┤
│                    核心层 (Core Layer)                      │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 模块关系

```
核心层 (Core)
├── 模块管理 (Module Management)
├── 配置管理 (Configuration Management)
├── 内存池管理 (Memory Pool Management)  ← 新增
└── 基础数据结构 (Basic Data Structures)

基础层 (Base)
├── 数据类型定义 (Data Type Definitions)
├── 系统调用封装 (System Call Wrappers)
├── 平台抽象 (Platform Abstraction)
├── 错误处理 (Error Handling)
└── 内存池模块 (Memory Pool Module)     ← 新增

事件层 (Event)
├── 事件循环 (Event Loop)
├── 事件管理 (Event Management)
├── 定时器管理 (Timer Management)
└── 信号处理 (Signal Handling)

网络层 (Network)
├── TCP 协议支持 (TCP Protocol Support)
├── UDP 协议支持 (UDP Protocol Support)
├── 网络工具函数 (Network Utility Functions)
└── 连接池管理 (Connection Pool Management)

系统层 (System)
├── 进程管理 (Process Management)
├── 线程管理 (Thread Management)
├── 文件系统 (File System)
└── 进程间通信 (IPC)

工具层 (Utility)
├── 日志系统 (Logging System)
├── 加密算法 (Cryptographic Algorithms)
├── 压缩算法 (Compression Algorithms)
├── 性能监控 (Performance Monitoring)
└── 调试工具 (Debugging Tools)
```

## 3. 核心层设计 (Core Layer)

### 3.1 模块管理系统

```c
typedef struct nv_module_s {
    nv_module_type_t type;           // 模块类型
    const char *name;                 // 模块名称
    int (*init)(void *ctx);          // 初始化函数
    int (*cleanup)(void *ctx);       // 清理函数
    void *private_data;              // 私有数据
} nv_module_t;
```

### 3.2 配置管理系统

```c
typedef struct nv_conf_s {
    int daemon;                      // 是否以守护进程运行
    int worker_processes;            // 工作进程数量
    int worker_connections;          // 每个进程最大连接数
    char *pid_file;                  // PID文件路径
    char *log_file;                  // 日志文件路径
    int log_level;                   // 日志级别
    void *modules[NV_MODULE_MAX];    // 模块配置数组
} nv_conf_t;
```

### 3.3 内存池管理

```c
typedef struct nv_pool_s {
    nv_pool_block_t *blocks;         // 内存块链表
    nv_pool_block_t *current;        // 当前内存块
    nv_pool_block_t *large;          // 大内存块链表
    size_t max_size;                 // 单个内存块最大大小
    size_t min_size;                 // 单个内存块最小大小
    nv_pool_stats_t stats;           // 统计信息
    int auto_extend;                 // 是否自动扩展
    int zero_clear;                  // 是否清零内存
    void *private_data;              // 私有数据
} nv_pool_t;
```

## 4. 基础层设计 (Base Layer)

### 4.1 内存池模块

内存池模块是基础层的重要组成部分，提供了高效的内存管理功能：

#### 内存池块结构
```c
typedef struct nv_pool_block_s {
    struct nv_pool_block_s *next;    // 下一个内存块
    size_t size;                     // 内存块大小
    size_t used;                     // 已使用大小
    char *start;                     // 内存块起始地址
    char *end;                       // 内存块结束地址
    char *current;                   // 当前可用地址
} nv_pool_block_t;
```

#### 内存池统计信息
```c
typedef struct nv_pool_stats_s {
    size_t total_blocks;             // 总内存块数
    size_t total_size;               // 总内存大小
    size_t used_size;                // 已使用内存大小
    size_t free_size;                // 空闲内存大小
    size_t allocation_count;         // 分配次数
    size_t deallocation_count;       // 释放次数
    size_t fragmentation_count;      // 碎片化次数
} nv_pool_stats_t;
```

#### 内存池配置
```c
typedef struct nv_pool_config_s {
    size_t initial_size;             // 初始内存池大小
    size_t max_size;                 // 最大内存池大小
    size_t min_size;                 // 最小内存块大小
    int auto_extend;                 // 是否自动扩展
    int zero_clear;                  // 是否清零内存
    int enable_stats;                // 是否启用统计
} nv_pool_config_t;
```

#### 主要功能特性
- **高性能内存分配**：预分配内存块，减少系统调用
- **智能内存管理**：支持自动扩展和内存碎片整理
- **内存对齐支持**：支持指定字节对齐的内存分配
- **详细统计信息**：提供完整的内存使用统计
- **调试和验证**：支持内存池完整性检查和调试信息

## 5. 事件层设计 (Event Layer)

### 5.1 事件循环架构

```c
typedef struct nv_loop_s {
    int epoll_fd;                    // epoll文件描述符
    int max_events;                  // 最大事件数量
    struct epoll_event *events;      // 事件数组
    int event_count;                 // 当前事件数量
    nv_loop_state_t state;          // 循环状态
    nv_pool_t *pool;                // 内存池
    nv_event_t *timer_events;       // 定时器事件
    nv_event_t *signal_events;      // 信号事件
    nv_event_t *idle_events;        // 空闲事件
    void *private_data;              // 私有数据
} nv_loop_t;
```

### 5.2 事件类型定义

```c
#define NV_EVENT_READ     0x0001     // 读事件
#define NV_EVENT_WRITE    0x0002     // 写事件
#define NV_EVENT_ERROR    0x0004     // 错误事件
#define NV_EVENT_HUP      0x0008     // 挂起事件
#define NV_EVENT_TIMER    0x0010     // 定时器事件
#define NV_EVENT_SIGNAL   0x0020     // 信号事件
```

## 6. 网络层设计 (Network Layer)

### 6.1 TCP 协议支持

```c
typedef struct nv_tcp_s {
    nv_handle_t handle;              // 基础句柄
    int socketfd;                    // 套接字文件描述符
    int port;                        // 端口号
    int family;                      // 地址族
    int type;                        // 套接字类型
    int protocol;                    // 协议
    int flags;                       // 标志位
    nv_tcp_state_t state;           // 连接状态
    struct nv_loop_s *loop;         // 事件循环
    nv_event_t read_event;          // 读事件
    nv_event_t write_event;         // 写事件
    nv_pool_t *pool;                // 内存池
    struct sockaddr_in local_addr;   // 本地地址
    struct sockaddr_in remote_addr;  // 远程地址
    void *user_data;                // 用户数据
} nv_tcp_t;
```

### 6.2 UDP 协议支持

```c
typedef struct nv_udp_s {
    nv_handle_t handle;              // 基础句柄
    int socketfd;                    // 套接字文件描述符
    int port;                        // 端口号
    int family;                      // 地址族
    int type;                        // 套接字类型
    int protocol;                    // 协议
    int flags;                       // 标志位
    nv_udp_state_t state;           // 连接状态
    struct nv_loop_s *loop;         // 事件循环
    nv_event_t read_event;          // 读事件
    nv_event_t write_event;         // 写事件
    nv_pool_t *pool;                // 内存池
    struct sockaddr_in src_addr;     // 源地址
    struct sockaddr_in dest_addr;    // 目标地址
    struct sockaddr_in multicast_addr; // 多播地址
    void *user_data;                // 用户数据
} nv_udp_t;
```

## 7. 系统层设计 (System Layer)

### 7.1 进程管理

- 进程创建和销毁
- 进程间通信 (IPC)
- 信号处理
- 进程监控

### 7.2 线程管理

- 线程创建和销毁
- 线程池管理
- 线程同步 (互斥锁、条件变量、信号量)
- 线程安全

### 7.3 文件系统

- 文件操作封装
- 目录操作
- 文件监控
- 异步I/O

## 8. 工具层设计 (Utility Layer)

### 8.1 日志系统

```c
#define NV_LOG_EMERG   0   // 系统不可用
#define NV_LOG_ALERT   1   // 必须立即采取行动
#define NV_LOG_CRIT    2   // 严重情况
#define NV_LOG_ERR     3   // 错误情况
#define NV_LOG_WARN    4   // 警告情况
#define NV_LOG_NOTICE  5   // 正常但重要的情况
#define NV_LOG_INFO    6   // 信息性消息
#define NV_LOG_DEBUG   7   // 调试级别消息
```

### 8.2 性能监控

```c
typedef struct nv_perf_counter_s {
    const char *name;                // 计数器名称
    unsigned long count;             // 调用次数
    unsigned long total_time;        // 总耗时
    unsigned long min_time;          // 最小耗时
    unsigned long max_time;          // 最大耗时
    unsigned long start_time;        // 开始时间
} nv_perf_counter_t;
```

## 9. 内存池模块集成

### 9.1 模块注册

内存池模块通过以下方式注册到核心系统：

```c
NV_MEMORY_POOL_MODULE_REGISTER(memory_pool, 
                               nv_memory_pool_init, 
                               nv_memory_pool_cleanup);
```

### 9.2 与其他模块的集成

- **事件循环**：使用内存池管理事件相关内存
- **网络连接**：使用内存池管理连接缓冲区
- **日志系统**：使用内存池管理日志缓冲区
- **性能监控**：使用内存池管理监控数据结构

### 9.3 内存池使用示例

```c
#include <nv_pool.h>

int main() {
    // 创建内存池
    nv_pool_config_t config = NV_POOL_CONFIG_DEFAULT;
    nv_pool_t *pool = nv_pool_create(&config);
    
    // 分配内存
    char *buffer = nv_pool_alloc(pool, 1024);
    if (buffer) {
        strcpy(buffer, "Hello, Memory Pool!");
        printf("%s\n", buffer);
    }
    
    // 打印统计信息
    nv_pool_print_stats(pool);
    
    // 清理
    nv_pool_destroy(pool);
    return 0;
}
```

## 10. 设计原则

### 10.1 模块化设计

- 每个功能模块独立，职责单一
- 模块间通过标准接口通信
- 支持动态加载和卸载
- 内存池模块作为基础服务提供者

### 10.2 高性能设计

- 事件驱动架构
- 异步I/O操作
- 内存池管理
- 零拷贝技术

### 10.3 可扩展性

- 插件化架构
- 配置驱动
- 支持多种协议
- 跨平台兼容

### 10.4 易用性

- 简洁的API设计
- 丰富的示例代码
- 完善的文档说明
- 错误处理机制

## 11. 使用示例

### 11.1 基本使用流程

```c
#include <nv.h>

int main() {
    // 初始化库
    nv_library_init();
    
    // 创建事件循环
    struct nv_loop_s loop;
    nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
    nv_loop_init(&loop, &config);
    
    // 创建TCP服务器
    nv_tcp_server_t server;
    nv_tcp_server_create(&server, &loop, 8080);
    
    // 启动服务器
    nv_tcp_server_start(&server);
    
    // 运行事件循环
    nv_loop_run(&loop);
    
    // 清理资源
    nv_tcp_server_destroy(&server);
    nv_loop_cleanup(&loop);
    nv_library_cleanup();
    
    return 0;
}
```

### 11.2 TCP服务器示例

```c
void connection_handler(struct nv_tcp_s *conn) {
    nv_log_info("New connection established");
}

void data_handler(struct nv_tcp_s *conn, const char *data, size_t len) {
    nv_log_info("Received %zu bytes", len);
    // 处理数据
    nv_tcp_write(conn, data, len);
}

void close_handler(struct nv_tcp_s *conn) {
    nv_log_info("Connection closed");
}

int main() {
    struct nv_loop_s loop;
    nv_loop_init(&loop, NULL);
    
    nv_tcp_server_t server;
    server.connection_handler = connection_handler;
    server.data_handler = data_handler;
    server.close_handler = close_handler;
    
    nv_tcp_server_create(&server, &loop, 8080);
    nv_tcp_server_start(&server);
    
    nv_loop_run(&loop);
    
    return 0;
}
```

### 11.3 内存池使用示例

```c
#include <nv_pool.h>

int main() {
    // 创建内存池
    nv_pool_t *pool = nv_pool_create_default();
    
    // 分配不同类型的内存
    char *str = nv_pool_alloc(pool, 100);
    int *numbers = nv_pool_alloc_array(pool, int, 1000);
    test_data_t *data = nv_pool_alloc_type(pool, test_data_t);
    
    // 使用内存...
    
    // 打印统计信息
    nv_pool_print_stats(pool);
    
    // 清理
    nv_pool_destroy(pool);
    return 0;
}
```

## 12. 编译和构建

### 12.1 依赖要求

- Linux 内核 3.0+
- GCC 4.8+ 或 Clang 3.3+
- CMake 3.10+
- 支持 epoll 的 Linux 系统

### 12.2 编译步骤

```bash
mkdir build && cd build
cmake ..
make
make install
```

### 12.3 配置选项

```bash
cmake -DENABLE_DEBUG=ON -DENABLE_TESTS=ON -DENABLE_MEMORY_POOL=ON ..
```

## 13. 性能特性

### 13.1 高并发支持

- 单进程支持数万并发连接
- 事件驱动，非阻塞I/O
- 高效的内存管理
- 内存池优化

### 13.2 低延迟

- 零拷贝数据传输
- 优化的网络栈
- 最小化系统调用
- 内存池预分配

### 13.3 高吞吐量

- 异步I/O操作
- 批量处理机制
- 智能缓冲区管理
- 内存池自动扩展

## 14. 未来规划

### 14.1 功能扩展

- HTTP/HTTPS 协议支持
- WebSocket 支持
- 数据库连接池
- 缓存系统
- 高级内存池功能

### 14.2 性能优化

- 多核CPU优化
- NUMA架构支持
- 硬件加速支持
- 内存管理优化
- 内存池碎片整理

### 14.3 平台支持

- Windows 平台支持
- macOS 平台支持
- 嵌入式系统支持
- 云原生支持

## 15. 总结

libnv 是一个设计精良、功能完整的高性能基础库，采用现代化的架构设计理念，为开发者提供了强大而灵活的基础设施支持。通过模块化设计、事件驱动架构、异步I/O和内存池管理等特性，libnv 能够满足高性能网络应用的需求，同时保持良好的可维护性和可扩展性。

### 新增的内存池模块特性

- **高效内存管理**：预分配内存块，减少系统调用开销
- **智能扩展**：支持自动扩展和手动扩展
- **内存对齐**：支持指定字节对齐的内存分配
- **详细统计**：提供完整的内存使用统计信息
- **调试支持**：支持内存池完整性检查和调试信息

该库特别适合用于：

- 高性能网络服务器
- 实时通信应用
- 物联网设备
- 嵌入式系统
- 云原生应用

通过持续的功能完善和性能优化，libnv 将成为 Linux 平台上不可或缺的基础库之一。
