# Makefile for NV Event Loop Demo
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
INCLUDES = -I../src -I../src/base -I../src/event
LIBS = -lpthread -lrt

# 源文件
EVENT_LOOP_SRCS = event_loop_demo.c ../src/event/nv_event.c ../src/event/nv_loop.c
EVENT_LOOP_OBJS = $(EVENT_LOOP_SRCS:.c=.o)

ADVANCED_EVENT_LOOP_SRCS = event_loop_advanced_demo.c ../src/event/nv_event.c ../src/event/nv_loop.c
ADVANCED_EVENT_LOOP_OBJS = $(ADVANCED_EVENT_LOOP_SRCS:.c=.o)

NETWORK_DEMO_SRCS = network_demo.c ../src/event/nv_event.c ../src/event/nv_loop.c ../src/event/nv_tcp.c ../src/event/nv_udp.c
NETWORK_DEMO_OBJS = $(NETWORK_DEMO_SRCS:.c=.o)

# 目标文件
TARGETS = event_loop_demo event_loop_advanced_demo network_demo

# 默认目标
all: $(TARGETS)

# 基础事件循环示例
event_loop_demo: $(EVENT_LOOP_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 高级事件循环示例
event_loop_advanced_demo: $(ADVANCED_EVENT_LOOP_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 网络示例
network_demo: $(NETWORK_DEMO_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理
clean:
	rm -f $(TARGETS) $(EVENT_LOOP_OBJS) $(ADVANCED_EVENT_LOOP_OBJS) $(NETWORK_DEMO_OBJS)

# 安装
install: all
	@echo "Installing event loop demos..."
	@echo "Use 'make run-basic', 'make run-advanced', or 'make run-network' to run the demos"

# 运行基础示例
run-basic: event_loop_demo
	./event_loop_demo

# 运行高级示例
run-advanced: event_loop_advanced_demo
	./event_loop_advanced_demo

# 运行网络示例
run-network: network_demo
	./network_demo

# 运行所有示例
run: run-basic run-advanced run-network

# 帮助
help:
	@echo "Available targets:"
	@echo "  all              - Build all targets"
	@echo "  clean            - Remove build artifacts"
	@echo "  install          - Install (placeholder)"
	@echo "  run-basic        - Run the basic event loop demo"
	@echo "  run-advanced     - Run the advanced event loop demo"
	@echo "  run-network      - Run the network demo"
	@echo "  run              - Run all demos"
	@echo "  help             - Show this help message"

.PHONY: all clean install run run-basic run-advanced run-network help
