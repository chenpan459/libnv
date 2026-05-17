#!/bin/bash
# libnv CMake 构建脚本

set -e

RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

clean_code() {
    echo -e "${CYAN}清理构建目录 ${BUILD_DIR}...${NC}"
    rm -rf "${BUILD_DIR}"
    echo -e "${CYAN}清理成功。${NC}"
}

compile_code() {
    echo -e "${CYAN}配置 CMake (${BUILD_TYPE})...${NC}"
    cmake -S . -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DNV_BUILD_APP=ON \
        -DNV_BUILD_EXAMPLES="${NV_BUILD_EXAMPLES:-OFF}" \
        -DNV_BUILD_TESTS="${NV_BUILD_TESTS:-OFF}"

    echo -e "${CYAN}编译...${NC}"
    cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || echo 4)"

    if [ $? -ne 0 ]; then
        echo -e "${RED}编译失败。${NC}"
        exit 1
    fi
    echo -e "${CYAN}编译成功。产物: ${BUILD_DIR}/lib ${BUILD_DIR}/bin${NC}"
}

case "${1:-}" in
    clean)
        clean_code
        ;;
    rebuild)
        clean_code
        compile_code
        ;;
    *)
        compile_code
        ;;
esac

echo -e "${CYAN}完成。${NC}"
