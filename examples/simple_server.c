/************************************************
 * @文件名: simple_server.c
 * @功能: 简单的TCP服务器示例，展示libnv框架使用方法
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，实现简单TCP服务器
 ***********************************************/

#include <nv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 连接处理函数 */
void connection_handler(struct nv_tcp_s *conn) {
    nv_log_info("新连接建立: %s:%d", 
                inet_ntoa(conn->remote_addr.sin_addr), 
                ntohs(conn->remote_addr.sin_port));
}

/* 数据接收处理函数 */
void data_handler(struct nv_tcp_s *conn, const char *data, size_t len) {
    nv_log_info("接收到 %zu 字节数据", len);
    
    // 简单的回显服务
    char response[1024];
    snprintf(response, sizeof(response), "服务器收到: %.*s", (int)len, data);
    
    if (nv_tcp_write(conn, response, strlen(response)) < 0) {
        nv_log_error("发送响应失败");
    }
}

/* 连接关闭处理函数 */
void close_handler(struct nv_tcp_s *conn) {
    nv_log_info("连接关闭: %s:%d", 
                inet_ntoa(conn->remote_addr.sin_addr), 
                ntohs(conn->remote_addr.sin_port));
}

/* 信号处理函数 */
void signal_handler(int signo) {
    nv_log_info("收到信号 %d，准备退出", signo);
    exit(0);
}

int main(int argc, char *argv[]) {
    int port = 8080;
    
    // 解析命令行参数
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    nv_log_info("启动TCP服务器，端口: %d", port);
    
    // 初始化库
    if (nv_library_init() != NV_OK) {
        nv_log_error("库初始化失败");
        return -1;
    }
    
    // 设置日志级别
    nv_log_set_level(NV_LOG_INFO);
    
    // 设置信号处理
    nv_signal_set(SIGINT, signal_handler);
    nv_signal_set(SIGTERM, signal_handler);
    
    // 创建事件循环
    struct nv_loop_s loop;
    nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
    config.max_events = 1024;
    config.timeout_ms = 1000;
    
    if (nv_loop_init(&loop, &config) != NV_OK) {
        nv_log_error("事件循环初始化失败");
        nv_library_cleanup();
        return -1;
    }
    
    // 创建TCP服务器
    nv_tcp_server_t server;
    memset(&server, 0, sizeof(server));
    
    server.loop = &loop;
    server.port = port;
    server.backlog = 128;
    server.max_connections = 1000;
    server.connection_handler = connection_handler;
    server.data_handler = data_handler;
    server.close_handler = close_handler;
    
    if (nv_tcp_server_create(&server, &loop, port) != NV_OK) {
        nv_log_error("TCP服务器创建失败");
        nv_loop_cleanup(&loop);
        nv_library_cleanup();
        return -1;
    }
    
    // 启动服务器
    if (nv_tcp_server_start(&server) != NV_OK) {
        nv_log_error("TCP服务器启动失败");
        nv_tcp_server_destroy(&server);
        nv_loop_cleanup(&loop);
        nv_library_cleanup();
        return -1;
    }
    
    nv_log_info("TCP服务器启动成功，监听端口 %d", port);
    nv_log_info("按 Ctrl+C 退出");
    
    // 运行事件循环
    nv_loop_run(&loop);
    
    // 清理资源
    nv_log_info("正在清理资源...");
    nv_tcp_server_destroy(&server);
    nv_loop_cleanup(&loop);
    nv_library_cleanup();
    
    nv_log_info("服务器已退出");
    return 0;
}
