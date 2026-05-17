#!/bin/bash
# libnv CMake 构建脚本（必须指定版本号）

set -e

RED='\033[0;31m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

usage() {
    echo -e "${YELLOW}用法:${NC}"
    echo "  $0 <版本号>              编译（示例: $0 1.0.0）"
    echo "  $0 <版本号> clean        清理 build 目录"
    echo "  $0 <版本号> rebuild      清理后重新编译"
    echo ""
    echo -e "${YELLOW}环境变量:${NC} BUILD_DIR BUILD_TYPE NV_BUILD_EXAMPLES NV_BUILD_TESTS"
    echo ""
    echo -e "${YELLOW}示例:${NC}"
    echo "  $0 1.2.3"
    echo "  $0 2.0.0-beta rebuild"
}

validate_version() {
    local ver="$1"
    if [[ -z "$ver" ]]; then
        return 1
    fi
    if [[ ! "$ver" =~ ^[0-9]+(\.[0-9]+)*([.-][0-9A-Za-z._-]+)?$ ]]; then
        return 1
    fi
    return 0
}

clean_code() {
    echo -e "${CYAN}清理构建目录 ${BUILD_DIR}...${NC}"
    rm -rf "${BUILD_DIR}"
    echo -e "${CYAN}清理成功。${NC}"
}

compile_code() {
    local version="$1"
    echo -e "${CYAN}配置 CMake (${BUILD_TYPE}), 版本 ${version}...${NC}"
    cmake -S . -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DNV_VERSION="${version}" \
        -DNV_BUILD_APP=ON \
        -DNV_BUILD_EXAMPLES="${NV_BUILD_EXAMPLES:-OFF}" \
        -DNV_BUILD_TESTS="${NV_BUILD_TESTS:-OFF}"

    echo -e "${CYAN}编译...${NC}"
    cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || echo 4)"
    echo -e "${CYAN}编译成功。版本: ${version}  产物: ${BUILD_DIR}/lib ${BUILD_DIR}/bin${NC}"
}

# clean 可不带版本号；编译必须带版本号
if [[ "${1:-}" == "clean" && -z "${2:-}" ]]; then
    clean_code
    echo -e "${CYAN}完成。${NC}"
    exit 0
fi

NV_VERSION="${1:-}"
ACTION="${2:-build}"

if ! validate_version "${NV_VERSION}"; then
    echo -e "${RED}错误: 必须指定有效版本号。${NC}" >&2
    echo "" >&2
    usage
    exit 1
fi

case "${ACTION}" in
    clean)
        clean_code
        ;;
    rebuild)
        clean_code
        compile_code "${NV_VERSION}"
        ;;
    build|"")
        compile_code "${NV_VERSION}"
        ;;
    *)
        echo -e "${RED}未知操作: ${ACTION}${NC}" >&2
        usage
        exit 1
        ;;
esac

echo -e "${CYAN}完成。${NC}"
