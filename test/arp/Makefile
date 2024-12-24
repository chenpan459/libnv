CC = gcc

# 自动查找所有头文件所在的目录
INCLUDE_DIRS = $(shell find ../../src -type f -name '*.h' -exec dirname {} \; | sort -u)
CFLAGS = -Wall $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

#CFLAGS = -Wall -Wextra -I../../src
LDFLAGS = -L../../lib -lnv -pthread -lrt -lusb-1.0 -lm -ldl

TARGETS = gratuitous_arp arp_broadcast 

all: $(TARGETS)

# 生成 nv_tcp_cli 可执行文件
gratuitous_arp: gratuitous_arp.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# 生成 nv_tcp_srv 可执行文件
arp_broadcast: arp_broadcast.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


clean:
	rm -f $(TARGETS)

.PHONY: all clean
