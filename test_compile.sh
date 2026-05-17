#!/bin/bash
# 使用 CMake 快速验证核心模块能否编译

set -e

BUILD_DIR="${BUILD_DIR:-build-check}"

echo "Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DNV_BUILD_APP=ON \
    -DNV_BUILD_EXAMPLES=OFF \
    -DNV_BUILD_TESTS=OFF

echo "Building libnv..."
cmake --build "${BUILD_DIR}" --target nv_shared -j"$(nproc 2>/dev/null || echo 4)"

echo "Compilation test completed successfully."
