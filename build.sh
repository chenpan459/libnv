#!/bin/bash
# 定义颜色
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 编译代码的函数
compile_code() {
     echo -e "${CYAN}开始编译代码...${NC}"
    make
    if [ $? -ne 0 ]; then #检查上一个命令的退出状态码。如果状态码不为 0，表示编译失败，脚本将打印错误信息并终止运行。
         echo -e "${RED}编译失败，终止脚本运行。${NC}"
        exit 1
    fi
   echo -e "${CYAN}编译成功。${NC}"
}

# 清理代码的函数
clean_code() {
    echo -e "${CYAN}清理代码...${NC}"
    make clean
    if [ $? -ne 0 ]; then #检查上一个命令的退出状态码。如果状态码不为 0，表示清理失败，脚本将打印错误信息并终止运行。
         echo -e "${RED}清理失败，终止脚本运行。${NC}"
        exit 1
    fi
    echo -e "${CYAN}清理成功。${NC}"
}

# 主脚本逻辑
clean_code
compile_code

echo -e "${CYAN}脚本运行完成。${NC}"
