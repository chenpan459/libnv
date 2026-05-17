cmake_minimum_required(VERSION 3.10)

# 禁止在源码目录内配置/编译，所有 .o 等中间文件只生成在 build/ 下
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "禁止在源码目录内构建（会在 src/ 下产生 .o 文件）。\n"
        "请使用独立构建目录，例如：\n"
        "  cmake -S . -B build\n"
        "  cmake --build build")
endif()

project(libnv VERSION 1.0.0 LANGUAGES C)

# 版本号（build.sh 通过 -DNV_VERSION= 传入，直接 cmake 时也建议指定）
if(NOT DEFINED NV_VERSION OR "${NV_VERSION}" STREQUAL "")
    message(FATAL_ERROR
        "未指定版本号 NV_VERSION。\n"
        "请使用: ./build.sh <版本号>\n"
        "或:     cmake -S . -B build -DNV_VERSION=1.0.0")
endif()

string(STRIP "${NV_VERSION}" NV_VERSION)
set(NV_BUILD_INFO_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${NV_BUILD_INFO_DIR}")
string(TIMESTAMP NV_BUILD_TIME "%Y-%m-%d %H:%M:%S")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/include/nv_build_info.h.in"
    "${NV_BUILD_INFO_DIR}/nv_build_info.h"
    @ONLY
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 所有产物统一输出到 build/ 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# 选项
option(NV_ENABLE_USB      "Build USB module (requires libusb-1.0)" ON)
option(NV_BUILD_STATIC    "Build static library libnv.a"           ON)
option(NV_BUILD_APP       "Build main process daemon (nvd)"        ON)
option(NV_BUILD_EXAMPLES  "Build examples (部分示例待修复)"        OFF)
option(NV_BUILD_TESTS     "Build tests (部分测试待修复)"           OFF)
option(NV_ENABLE_FEATURE "Enable ENABLE_FEATURE compile define"   ON)

if(NV_ENABLE_FEATURE)
    set(NV_COMPILE_DEFS ENABLE_FEATURE)
endif()

find_package(Threads REQUIRED)

# 核心库
add_subdirectory(src)

if(NOT NV_BUILD_STATIC)
    message(STATUS "NV_BUILD_STATIC=OFF: only building shared library")
    # nv_static 已在 src/CMakeLists.txt 创建；此处不强制删除，仅提示
endif()

# 主进程守护程序
if(NV_BUILD_APP)
    add_subdirectory(app)
endif()

# 示例
if(NV_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 测试程序
if(NV_BUILD_TESTS)
    add_subdirectory(test)
endif()

# 安装（可选）
include(GNUInstallDirs)
install(TARGETS nv_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
if(NV_BUILD_STATIC)
    install(TARGETS nv_static ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nv
    FILES_MATCHING PATTERN "*.h"
)

message(STATUS "")
message(STATUS "libnv ${NV_VERSION} (build @ ${NV_BUILD_TIME})")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  NV_ENABLE_USB:  ${NV_ENABLE_USB}")
message(STATUS "  NV_BUILD_APP:   ${NV_BUILD_APP}")
message(STATUS "  NV_BUILD_TESTS: ${NV_BUILD_TESTS}")
message(STATUS "  Build dir:      ${CMAKE_BINARY_DIR}")
message(STATUS "")

# 清理源码目录中遗留的 .o（旧 Makefile 在 src/ 内编译的产物）
add_custom_target(clean-src-objects
    COMMAND find "${CMAKE_SOURCE_DIR}/src" -name "*.o" -delete
    COMMAND find "${CMAKE_SOURCE_DIR}/src" -name "*.os" -delete
    COMMENT "Remove stray object files from source tree"
)
