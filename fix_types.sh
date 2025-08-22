#!/bin/bash

# 修复类型引用的脚本

echo "正在修复类型引用..."

# 修复 .c 文件中的 nv_loop_t 引用
find . -name "*.c" -type f -exec sed -i 's/nv_loop_t/struct nv_loop_s/g' {} \;

# 修复 .c 文件中的 nv_tcp_t 引用
find . -name "*.c" -type f -exec sed -i 's/nv_tcp_t/struct nv_tcp_s/g' {} \;

# 修复 .c 文件中的 nv_udp_t 引用
find . -name "*.c" -type f -exec sed -i 's/nv_udp_t/struct nv_udp_s/g' {} \;

echo "类型引用修复完成！"
