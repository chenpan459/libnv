# 定义编译器
CC = gcc

# 定义编译选项
CFLAGS = -Wall

# 定义链接选项
LDFLAGS = 

# 定义需要构建的子目录
SUBDIRS = src examples
#SUBDIRS = $(shell find . -maxdepth 1 -type d ! -name .)

# 默认目标
all: $(SUBDIRS)


# 清理构建的子目录
clean:
	for dir in $(SUBDIRS); do \
		if [ -f $$dir/Makefile ]; then \
		$(MAKE) -C $$dir clean; \
		fi \
		done

# 调用子目录的 Makefile
$(SUBDIRS):
	if [ -f $@/Makefile ]; then \
		$(MAKE) -C $@; \
	fi

# 防止 make 执行时自动补全文件名
.PHONY: all clean $(SUBDIRS)
