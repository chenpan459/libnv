# 定义编译器
CC = gcc

# 自动查找所有头文件所在的目录
INCLUDE_DIRS = $(shell find . -type f -name '*.h' -exec dirname {} \; | sort -u)

CFLAGS = -Wall $(foreach dir, $(INCLUDE_DIRS), -I$(dir))



# 定义链接选项
LDFLAGS = -pthread -lrt -ldl

# 定义库的名称
LIB_NAME = libnv

# 定义库的路径
LIB_PATH = ../lib

# 定义静态库文件的完整路径
STATIC_LIB_FILE = $(LIB_PATH)/$(LIB_NAME).a

# 定义动态库文件的完整路径
SHARED_LIB_FILE = $(LIB_PATH)/$(LIB_NAME).so

# 定义测试可执行文件的名称
TEST_PROG = program_test

# 自动查找 src 目录下的所有源文件
SRCS = $(shell find ./ -type f -name '*.c')

# 从源文件列表生成目标文件列表
OBJS = $(SRCS:.c=.o)
OBJS_SHARED = $(OBJS:.o=.os)

# 默认目标
all: $(STATIC_LIB_FILE) $(SHARED_LIB_FILE) 
#$(TEST_PROG)

# 编译静态库
$(STATIC_LIB_FILE): $(OBJS)
	@mkdir -p $(LIB_PATH)
	ar rcs $@ $^ 

# 编译动态库
$(SHARED_LIB_FILE): $(OBJS_SHARED)
	@mkdir -p $(LIB_PATH) 
	$(CC) -shared -o $@ $^ 

# 编译可执行文件
#$(TEST_PROG): $(wildcard ./test/*.c) $(SHARED_LIB_FILE)
#	$(CC) $(CFLAGS) $(LDFLAGS) -L$(LIB_PATH) -lnv -o $@ ./test/*.c

# 编译 .c 文件为 .o 文件的规则（静态库）
%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

# 编译 .c 文件为 .os 文件的规则（动态库）
%.os: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -c $< -o $@

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(OBJS_SHARED) $(STATIC_LIB_FILE) $(SHARED_LIB_FILE) $(TEST_PROG)
	rm -rf $(LIB_PATH)
	@find . -name '*.o' -type f -delete
	@find . -name '*.os' -type f -delete

# 防止 make 执行时自动补全文件名
.PHONY: all clean