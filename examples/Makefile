# 定义编译器
CC = gcc

# 定义编译选项
CFLAGS = -Wall

# 定义链接选项
LDFLAGS = 

# 定义需要构建的子目录
SUBDIRS = ex0 ex1

# 默认目标
all: $(SUBDIRS)

# 清理构建的子目录
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# 调用子目录的 Makefile
$(SUBDIRS):
	$(MAKE) -C $@

# 防止 make 执行时自动补全文件名
.PHONY: all clean $(SUBDIRS)
