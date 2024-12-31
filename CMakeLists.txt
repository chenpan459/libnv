cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(libnv VERSION 1.0 LANGUAGES C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 包含源文件目录
include_directories(${PROJECT_SOURCE_DIR}/src)

# 查找所有源文件
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

# 添加动态库
add_library(nv SHARED ${SOURCES})

# 设置目标属性以确保 CMake 知道链接语言
set_target_properties(nv PROPERTIES LINKER_LANGUAGE C)


# 添加可执行文件
add_executable(arp_aging_message ${PROJECT_SOURCE_DIR}/test/arp/arp_aging_message.c)

# 链接动态库到可执行文件
#target_link_libraries(arp_aging_message nv)