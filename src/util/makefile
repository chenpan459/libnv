# 定义编译器
CC = gcc

# 定义编译选项
CFLAGS = -Wall -g

# 定义链接选项
LDFLAGS = -lpthread -lrt -lusb-1.0

# 找出所有的.c文件
SRCS = $(wildcard *.c)

# 定义目标文件名
TARGET = output

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGET) *.o

# 伪目标.PHONY
.PHONY: all clean
