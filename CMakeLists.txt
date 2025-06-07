cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(libnv VERSION 1.0 LANGUAGES C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)


# 递归查找所有头文件
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/src/*.h")

# 打印 HEADER_FILES 变量内容
#message("HEADER_FILES: ${HEADER_FILES}")

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/src)

# 包含源文件目录
include_directories(${PROJECT_SOURCE_DIR}/src)

# 查找所有源文件
#file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

# 添加动态库
add_library(nv SHARED ${SOURCES} ${HEADER_FILES})

# 设置目标属性以确保 CMake 知道链接语言
set_target_properties(nv PROPERTIES LINKER_LANGUAGE C)


#################### 定义宏开关###############################
add_definitions(-DENABLE_FEATURE)
# 或者使用 target_compile_definitions
# target_compile_definitions(nv PRIVATE ENABLE_FEATURE)
add_definitions(-D_GNU_SOURCE)

##############################################################

###################### 定义选项开关##########################
#option(ENABLE_GMT_0105_RNG "Enable GM/T 0105 Software RNG" OFF)
## 根据选项开关定义宏
#if(ENABLE_GMT_0105_RNG)
#    add_definitions(-DENABLE_GMT_0105_RNG)
#    # 或者使用 target_compile_definitions
#    # target_compile_definitions(nv PRIVATE ENABLE_GMT_0105_RNG)
#endif()
###################################################################

# 添加可执行文件
add_executable(arp_aging_message ${PROJECT_SOURCE_DIR}/test/arp/arp_aging_message.c)

# 链接动态库到可执行文件
#target_link_libraries(arp_aging_message nv)