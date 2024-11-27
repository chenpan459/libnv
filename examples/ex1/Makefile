CC = gcc

# 自动查找所有头文件所在的目录
INCLUDE_DIRS = $(shell find ../../src -type f -name '*.h' -exec dirname {} \; | sort -u)
CFLAGS = -Wall $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

#CFLAGS = -Wall -Wextra -I../../src
LDFLAGS = -L../../lib -lnv -pthread -lrt -lusb-1.0 -lm -ldl

TARGET = test_log

all: $(TARGET)

$(TARGET): nv_all_test.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
