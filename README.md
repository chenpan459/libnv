# libnv
linux 平台基础库,包含网络,文件,内存,日志等基础功能

编译环境ubuntu24.04

编译方法（仅 CMake）：

```bash
# 方式一：脚本（必须指定版本号）
./build.sh 1.0.0

# 方式二：手动
cmake -S . -B build -DNV_VERSION=1.0.0 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 运行主进程示例
./build/bin/nvd -f -c etc/nv.conf
```

常用选项：

```bash
cmake -S . -B build \
  -DNV_BUILD_APP=ON \          # 编译 nvd 守护进程（默认 ON）
  -DNV_BUILD_EXAMPLES=OFF \    # 编译 examples（默认 OFF）
  -DNV_BUILD_TESTS=OFF \       # 编译 test（默认 OFF）
  -DNV_ENABLE_USB=ON           # USB 模块（需 libusb）
```

产物目录：`build/lib/libnv.so`、`build/bin/nvd`

清理：`./build.sh clean` 或 `rm -rf build`

**注意**：必须在独立 `build/` 目录编译，禁止在源码目录执行 `cmake .`（会在 `src/` 下产生 `.o`）。清理遗留产物：`cmake --build build --target clean-src-objects`

依赖（可选 USB）：

```bash
sudo apt update
sudo apt install build-essential cmake libusb-1.0-0-dev
```