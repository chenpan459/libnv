# libnv

面向 Linux 的 C 语言基础库与守护进程框架，提供事件驱动 I/O、异步日志、网络、IPC、数据结构封装，以及 JSON / XML / SQLite 等常用能力。示例守护进程为 `nvd`。

**编译环境**：Ubuntu 24.04 · GCC · CMake 3.10+

更详细的模块说明见 [ARCHITECTURE.md](ARCHITECTURE.md)。

---

## 整体架构

libnv 采用**分层 + 模块化**设计：底层是可独立使用的 `libnv.so` / `libnv.a`，上层 `nv_core` 提供守护进程生命周期与可插拔业务钩子。

```
┌──────────────────────────────────────────────────────────────┐
│  应用层          app/nvd.c  (+ 用户 on_business_* 钩子)       │
├──────────────────────────────────────────────────────────────┤
│  核心层 (core)   启动 / 配置 / 事件循环 / Worker / CLI / 健康检查 │
├──────────────────────────────────────────────────────────────┤
│  事件层 (event)  epoll 循环 · TCP/UDP · 定时器 · signalfd       │
├──────────────────────────────────────────────────────────────┤
│  日志层 (log)    异步 MPSC 队列 · 专用写盘线程 · syslog         │
├──────────────────────────────────────────────────────────────┤
│  基础层 (base)   网络 · 系统 · IPC · 算法 · 数据结构 · 硬件(可选) │
├──────────────────────────────────────────────────────────────┤
│  数据封装        nv_json · nv_xml · nv_sqlite (+ third_party)   │
└──────────────────────────────────────────────────────────────┘
```

### 各层职责

| 层次 | 目录 | 说明 |
|------|------|------|
| **应用** | `app/` | `nvd` 入口，调用 `nv_core_run()` 并注册业务钩子 |
| **核心** | `src/core/` | 参数解析、INI 配置、守护进程化、信号/PID、主循环、优雅退出 |
| **事件** | `src/event/` | `nv_loop`（epoll）、`nv_tcp` / `nv_udp`、事件节点生命周期管理 |
| **日志** | `src/log/` | 分级日志、无锁入队、后台线程落盘，可配置队列与溢出策略 |
| **基础** | `src/base/` | 套接字、线程池、消息队列、内存池、INI、各类容器与工具函数 |
| **数据** | `src/base/data/` | `nv_json` / `nv_xml` / `nv_sqlite` 对第三方库的薄封装 |
| **第三方** | `third_party/` | cJSON、mxml、SQLite amalgamation（离线可构建） |

### 核心模块（`src/core/`）

主进程逻辑已按职责拆分为多个源文件，由 `nv_core_run()` 统一调度：

| 文件 | 职责 |
|------|------|
| `nv_core_run.c` | 统一入口 `nv_core_run()`、主循环、退出与超时 |
| `nv_core_startup.c` | 日志启动、PID 文件、信号注册、rlimit、systemd notify |
| `nv_core_args.c` | 命令行默认值与 `getopt` 解析 |
| `nv_core_config.c` | 加载 `etc/nv.conf`（INI）并覆盖运行时选项 |
| `nv_core.c` | 业务初始化/清理（事件循环、线程池、消息队列、模块注册） |
| `nv_core_worker.c` | 多进程 Worker（fork）与崩溃拉起 |
| `nv_core_cli.c` | 可注册命令的 CLI 框架（`help` / 执行 / Tab） |
| `nv_core_telnet.c` | Telnet 管理口（可选，常量时间密码比对） |
| `nv_core_ctl.c` | Unix 域控制套接字 |
| `nv_core_health.c` | 运行时统计与心跳 |
| `nv_core_modules.c` | 内置模块注册（事件 tick、TCP/UDP 池等） |

### 守护进程生命周期

```
nv_core_run(ctx, argc, argv, hooks)
  │
  ├─ 1. 启动 (STARTUP)     解析参数 → 读配置 → 可选 daemonize → 日志/PID/信号
  ├─ 2. 业务 (BUSINESS)    hooks->on_business_init → epoll/signalfd/线程池/MQ/模块
  ├─ 3. 主循环 (LOOP)      nv_loop_run → IO / 定时器 / idle → hooks->on_idle
  ├─ 4. 退出 (SHUTDOWN)    停止循环 → 释放资源 → 删 PID → 停日志
  └─ 5. 异常 (EXCEPTION)   致命信号仅在 handler 置位，主线程做清理后退出
```

配置文件为 INI（`[section]`、`key = value`），示例见 `etc/nv.conf`。默认路径 `/etc/nv/nv.conf`，可用 `-c` 指定。

| 信号 | 行为 |
|------|------|
| SIGINT / SIGTERM | 请求优雅退出 |
| SIGHUP | 重载配置（`hooks->on_reload`） |
| SIGUSR1 | 退出并 re-exec 重启 |

### 数据与序列化封装

| API | 底层 | 能力 |
|-----|------|------|
| `nv_json_*` | cJSON 1.7.18 | 解析、组包、属性读写 |
| `nv_xml_*` | Mini-XML 4.0.3 | 解析、修改、保存文件/字符串 |
| `nv_sqlite_*` | SQLite 3.45.1 | 打开库、执行 SQL、预编译语句、事务 |

统一包含于 `src/include/nv.h`（SQLite 需 `NV_HAVE_SQLITE`）。

### 设计要点

- **事件驱动**：单线程 epoll 处理 IO，配合线程池处理耗时任务。
- **业务线程不阻塞写日志**：MPSC 无锁队列 + 独立 `nv-log` 线程。
- **可扩展 CLI**：`NV_CORE_CLI_REGISTER` 宏注册子命令，Telnet/ctl 共用同一注册表。
- **嵌入式友好**：第三方库 vendored，CMake 可选关闭 USB / SQLite。

### 嵌入式量产能力（P0/P1/P2）

| 能力 | 配置段 / API | 说明 |
|------|----------------|------|
| 嵌入式构建预设 | `NV_EMBEDDED_PROFILE=ON` 或 `./build.sh 1.0.0 embedded` | 关闭 SQLite/USB，Release 加 `-Os` |
| 收紧默认参数 | `etc/nv.conf.embedded` | threads=1、connections=128、默认关 Telnet |
| loadlib 白名单 | `[loadlib] plugin_dir` | 仅允许插件目录下 `.so`；`allow_absolute` 开发用 |
| 看门狗 | `[watchdog]`、`nv_watchdog_*` | 心跳/idle 喂 `/dev/watchdog` 或自定义 `cmd` |
| 日志轮转 | `[log] rotate_max_mb` | 按大小轮转，减轻 eMMC 写满风险 |
| SQLite 嵌入式 | `nv_sqlite_open_embedded()` | WAL + synchronous=NORMAL |
| 运行指标 pubsub | `[metrics] publish` | 心跳发布 JSON 到 `nv/metrics` 主题 |
| OTA 窗口 | `hooks.on_before_restart` | SIGUSR1 re-exec 前回调，可取消重启 |
| 崩溃摘要 | `[daemon] tombstone_file` | 致命信号时追加写入 tombstone |

推荐量产配置：复制 `etc/nv.conf.embedded` 到 `/etc/nv/nv.conf`。

### 模块体积参考（顺序量级）

| 组件 | 约 Flash（Release） | 裁剪建议 |
|------|---------------------|----------|
| libnv 核心 + event + log | 数百 KB～1MB | 静态链接、strip |
| + cJSON / mxml | +100～300KB | 不用 XML 可仍链入（或拆模块） |
| + SQLite amalgamation | +500KB～1MB+ | `NV_ENABLE_SQLITE=OFF` |
| + USB | 视 libusb | `NV_ENABLE_USB=OFF` |

---

## 目录结构

```
libnv/
├── app/                 # nvd 守护进程入口
├── etc/                 # 示例配置 nv.conf
├── src/
│   ├── include/         # nv.h、nv_config.h、构建信息模板
│   ├── core/            # 主进程框架
│   ├── event/           # 事件循环与 TCP/UDP
│   ├── log/             # 日志与 errno
│   ├── base/            # 基础库（net/sys/ipc/data/algo/hw…）
│   └── unix/            # 平台相关（共享内存等）
├── third_party/
│   ├── cjson/           # cJSON
│   ├── mxml/            # Mini-XML
│   └── sqlite/          # SQLite amalgamation
├── test/unit/           # 单元测试（ctest）
├── examples/            # 示例（默认不编译）
├── CMakeLists.txt
├── build.sh
└── ARCHITECTURE.md      # 扩展架构说明
```

---

## 编译

```bash
# 方式一：脚本（必须指定版本号）
./build.sh 1.0.0

# 方式二：手动
cmake -S . -B build -DNV_VERSION=1.0.0 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 运行守护进程（前台 + 指定配置；共享库需在 LD_LIBRARY_PATH 或 RPATH 中）
LD_LIBRARY_PATH=build/lib ./build/bin/nvd -f -c etc/nv.conf
```

`third_party/` 下各组件在 `build/lib/` 中单独产出静态库与动态库（`libnv` 通过链接依赖使用，不再把源码编入 `libnv`）：

| 组件 | 静态库 | 动态库 |
|------|--------|--------|
| cJSON | `libcjson.a` | `libcjson.so` |
| Mini-XML | `libmxml.a` | `libmxml.so` |
| SQLite（`NV_ENABLE_SQLITE=ON`） | `libsqlite.a` | `libsqlite.so` |

### 常用 CMake 选项

| 选项 | 默认 | 说明 |
|------|------|------|
| `NV_VERSION` | （必填） | 库版本号，写入 `nv_build_info.h` |
| `NV_BUILD_APP` | ON | 编译 `nvd` |
| `NV_BUILD_STATIC` | ON | 同时生成 `libnv.a` |
| `NV_BUILD_TESTS` | ON | 单元测试 + `ctest` |
| `NV_BUILD_EXAMPLES` | OFF | 示例程序 |
| `NV_ENABLE_USB` | ON | USB 模块（需 libusb-1.0，否则自动排除） |
| `NV_ENABLE_SQLITE` | ON | 内嵌 SQLite；OFF 时不编译 `nv_sqlite` |
| `NV_EMBEDDED_PROFILE` | OFF | ON 时嵌入式预设（关 SQLite/USB、-Os） |

```bash
# 嵌入式发行构建
./build.sh 1.0.0 embedded

cmake -S . -B build \
  -DNV_VERSION=1.0.0 \
  -DNV_BUILD_TESTS=ON \
  -DNV_EMBEDDED_PROFILE=ON \
  -DNV_ENABLE_SQLITE=OFF \
  -DNV_ENABLE_USB=OFF
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

**产物**：`build/lib/libnv.so`、`build/lib/libnv.a`、`build/bin/nvd`

**注意**：必须在独立 `build/` 目录编译，禁止在源码目录执行 `cmake .`（会在 `src/` 下产生 `.o`）。清理遗留产物：

```bash
cmake --build build --target clean-src-objects
# 或
./build.sh clean
```

### 依赖

```bash
sudo apt update
sudo apt install build-essential cmake

# 可选：USB 模块
sudo apt install libusb-1.0-0-dev
```

JSON / XML / SQLite 已 vendored，无需额外安装系统包。

---

## 快速接入

```c
#include <nv_core.h>

static int my_init(nv_core_ctx_t *ctx)
{
    return nv_core_business_init(ctx);  /* 或自定义初始化 */
}

int main(int argc, char **argv)
{
    nv_core_ctx_t   ctx;
    nv_core_hooks_t hooks = {
        .on_business_init    = my_init,
        .on_business_cleanup = nv_core_business_cleanup,
        .on_idle             = NULL,
        .on_reload           = NULL,
        .on_before_restart   = NULL,  /* OTA：SIGUSR1 前准备，返回非 NV_OK 取消 re-exec */
    };
    memset(&ctx, 0, sizeof(ctx));
    return nv_core_run(&ctx, argc, argv, &hooks) == NV_OK ? 0 : 1;
}
```

仅使用基础库（不跑守护进程框架）时，包含 `nv.h` 并链接 `-lnv` 即可使用各 `nv_*` 模块。

---

## 许可证

各 `third_party/` 组件遵循其各自许可证（cJSON: MIT，mxml: Apache-2.0，SQLite: Public Domain）。项目其余代码以仓库内声明为准。
