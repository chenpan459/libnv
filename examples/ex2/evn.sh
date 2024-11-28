#!/bin/bash

# 设置共享库路径
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../lib

# 打印 LD_LIBRARY_PATH 以验证设置
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"