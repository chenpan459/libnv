# 设置交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

# 设置编译器
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

# 设置查找路径
set(CMAKE_FIND_ROOT_PATH /opt/mips-gcc720-glibc229)

# 设置查找路径模式
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#################################################################
##########cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm.cmake ..
#########################################################################