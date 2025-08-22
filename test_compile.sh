#!/bin/bash

echo "Testing compilation of TCP and UDP modules..."

# 测试TCP模块编译
echo "Testing TCP module compilation..."
gcc -c -I./src -I./src/core -I./src/event src/event/nv_tcp.c -o /tmp/nv_tcp_test.o
if [ $? -eq 0 ]; then
    echo "✓ TCP module compiles successfully"
else
    echo "✗ TCP module compilation failed"
fi

# 测试UDP模块编译
echo "Testing UDP module compilation..."
gcc -c -I./src -I./src/core -I./src/event src/event/nv_udp.c -o /tmp/nv_udp_test.o
if [ $? -eq 0 ]; then
    echo "✓ UDP module compiles successfully"
else
    echo "✗ UDP module compilation failed"
fi

# 清理测试文件
rm -f /tmp/nv_tcp_test.o /tmp/nv_udp_test.o

echo "Compilation test completed."
