# NV Event Loop 事件循环

## 概述

NV Event Loop 是一个高性能的事件驱动架构，基于 Linux epoll 实现，支持 IO 事件、定时器事件、信号事件和空闲事件的处理。

## 架构特性

### 1. 事件类型
- **IO 事件**: 基于 epoll 的文件描述符事件（读、写、错误、挂起）
- **定时器事件**: 毫秒级精度的定时器，支持一次性或周期性
- **信号事件**: 系统信号处理（待实现）
- **空闲事件**: 在没有其他事件时的处理机会

### 2. 核心组件
- **nv_event_t**: 事件结构，包含事件属性和处理函数
- **nv_loop_t**: 事件循环，管理所有事件的生命周期
- **nv_loop_config_t**: 事件循环配置，可定制各种参数

### 3. 性能特性
- 基于 epoll 的高效事件通知
- 智能超时计算，避免不必要的唤醒
- 事件优先级支持
- 统计信息收集

## API 参考

### 事件循环管理

#### nv_loop_init
```c
int nv_loop_init(nv_loop_t *loop, const nv_loop_config_t *config);
```
初始化事件循环。

**参数:**
- `loop`: 事件循环结构指针
- `config`: 配置参数，NULL 使用默认配置

**返回值:**
- 0: 成功
- -1: 失败

#### nv_loop_run
```c
int nv_loop_run(nv_loop_t *loop);
```
运行事件循环，阻塞直到停止。

#### nv_loop_stop
```c
int nv_loop_stop(nv_loop_t *loop);
```
停止事件循环。

#### nv_loop_cleanup
```c
int nv_loop_cleanup(nv_loop_t *loop);
```
清理事件循环资源。

### 事件管理

#### nv_event_init
```c
int nv_event_init(nv_event_t *ev, nv_event_handler_t handler, void *data);
```
初始化事件结构。

#### nv_loop_add_event
```c
int nv_loop_add_event(nv_loop_t *loop, nv_event_t *ev, int events);
```
添加 IO 事件到事件循环。

#### nv_loop_add_timer
```c
int nv_loop_add_timer(nv_loop_t *loop, nv_event_t *ev, unsigned long timeout_ms);
```
添加定时器事件。

#### nv_loop_add_idle
```c
int nv_loop_add_idle(nv_loop_t *loop, nv_event_t *ev);
```
添加空闲事件。

## 使用示例

### 基本用法

```c
#include <nv_loop.h>
#include <nv_event.h>

/* 事件处理函数 */
void my_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_t *event = (nv_event_t *)ev;
    printf("Event triggered!\n");
}

int main() {
    nv_loop_t loop;
    nv_event_t event;
    
    /* 初始化事件循环 */
    nv_loop_init(&loop, NULL);
    
    /* 初始化事件 */
    NV_EVENT_INIT(&event, my_handler, NULL);
    nv_event_set_fd(&event, STDIN_FILENO);
    nv_event_set_events(&event, NV_EVENT_READ);
    
    /* 添加事件 */
    nv_loop_add_event(&loop, &event, NV_EVENT_READ);
    
    /* 运行事件循环 */
    nv_loop_run(&loop);
    
    /* 清理 */
    nv_loop_cleanup(&loop);
    
    return 0;
}
```

### 定时器示例

```c
void timer_handler(nv_loop_t *loop, void *ev, void *data) {
    static int count = 0;
    printf("Timer tick %d\n", ++count);
    
    /* 重新设置定时器实现周期性 */
    nv_event_t *event = (nv_event_t *)ev;
    nv_loop_add_timer(loop, event, 1000);
}

/* 在主函数中 */
nv_event_t timer_event;
NV_EVENT_INIT(&timer_event, timer_handler, NULL);
nv_loop_add_timer(&loop, &timer_event, 1000); /* 1秒定时器 */
```

### 配置选项

```c
nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
config.max_events = 256;        /* 最大事件数 */
config.timeout_ms = 100;        /* 默认超时 */
config.enable_timers = 1;       /* 启用定时器 */
config.enable_idle = 1;         /* 启用空闲事件 */
config.enable_signals = 1;      /* 启用信号处理 */

nv_loop_init(&loop, &config);
```

## 编译

### 依赖
- Linux 系统
- epoll 支持
- pthread 库
- rt 库（用于 clock_gettime）

### 编译命令
```bash
cd examples
make
```

### 运行示例
```bash
make run
```

## 性能优化

### 1. 事件数量
- 根据实际需求设置 `max_events`
- 避免过多的事件同时注册

### 2. 超时设置
- 合理设置定时器超时时间
- 使用空闲事件处理低优先级任务

### 3. 事件处理
- 事件处理函数应该快速返回
- 避免在事件处理中进行阻塞操作

## 注意事项

1. **线程安全**: 当前实现不是线程安全的，需要在单线程中使用
2. **资源管理**: 确保正确清理事件和事件循环
3. **错误处理**: 检查所有 API 调用的返回值
4. **信号处理**: 在事件循环外部设置信号处理器

## 扩展功能

### 待实现特性
- 多线程事件循环
- 事件优先级队列
- 信号事件处理
- 事件统计和监控
- 跨平台支持

### 自定义扩展
- 实现自定义事件类型
- 添加事件过滤器
- 集成日志系统
- 性能分析工具
