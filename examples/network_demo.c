/************************************************
 * @文件名: network_demo.c
 * @功能: 网络模块使用示例
 * @作者: chenpan
 * @日期: 2024-11-04
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "../src/event/nv_loop.h"
#include "../src/event/nv_event.h"
#include "../src/event/nv_tcp.h"
#include "../src/event/nv_udp.h"

/* 全局变量 */
static nv_loop_t main_loop;
static int running = 1;
static nv_tcp_server_t tcp_server;
static nv_udp_server_t udp_server;

/* 信号处理函数 */
void signal_handler(int sig) {
    printf("Received signal %d, stopping network demo...\n", sig);
    running = 0;
    nv_loop_stop(&main_loop);
}

/* TCP连接接受处理函数 */
void tcp_accept_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_ext_t *event = (nv_event_ext_t *)ev;
    nv_tcp_server_t *server = (nv_tcp_server_t *)data;
    
    printf("TCP accept event triggered\n");
    
    /* 接受新连接 */
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server->listen_fd, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_fd != -1) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("New TCP connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        
        /* 创建客户端连接 */
        nv_tcp_t *client = malloc(sizeof(nv_tcp_t));
        if (client) {
            memset(client, 0, sizeof(nv_tcp_t));
            client->fd = client_fd;
            client->state = NV_TCP_CONNECTED;
            client->loop = loop;
            
            /* 设置非阻塞模式 */
            nv_tcp_set_nonblocking(client);
            
            /* 创建读取事件 */
            nv_event_ext_t *read_event = malloc(sizeof(nv_event_ext_t));
            if (read_event) {
                NV_EVENT_INIT(read_event, tcp_read_handler, client);
                nv_event_set_fd(read_event, client_fd);
                nv_event_set_events(read_event, NV_EVENT_READ);
                
                if (nv_loop_add_event(loop, read_event, NV_EVENT_READ) == 0) {
                    client->read_event = read_event;
                    printf("TCP client read event added\n");
                } else {
                    free(read_event);
                    close(client_fd);
                    free(client);
                }
            }
        }
    }
}

/* TCP读取处理函数 */
void tcp_read_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_ext_t *event = (nv_event_ext_t *)ev;
    nv_tcp_t *client = (nv_tcp_t *)data;
    
    char buffer[1024];
    int bytes_read = nv_tcp_read(client, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("TCP received: %s\n", buffer);
        
        /* 回显数据 */
        nv_tcp_write(client, buffer, bytes_read);
    } else if (bytes_read == 0) {
        printf("TCP client disconnected\n");
        /* 清理连接 */
        nv_loop_del_event(loop, event);
        free(event);
        close(client->fd);
        free(client);
    } else {
        printf("TCP read error\n");
    }
}

/* UDP接收处理函数 */
void udp_recv_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_ext_t *event = (nv_event_ext_t *)ev;
    nv_udp_server_t *server = (nv_udp_server_t *)data;
    
    printf("UDP receive event triggered\n");
    
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int bytes_recv = recvfrom(server->fd, buffer, sizeof(buffer) - 1, 0,
                              (struct sockaddr*)&client_addr, &addr_len);
    
    if (bytes_recv > 0) {
        buffer[bytes_recv] = '\0';
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("UDP received from %s:%d: %s\n", 
               client_ip, ntohs(client_addr.sin_port), buffer);
        
        /* 回显数据 */
        sendto(server->fd, buffer, bytes_recv, 0,
               (struct sockaddr*)&client_addr, addr_len);
    }
}

/* 客户端测试函数 */
void *client_test_thread(void *arg) {
    sleep(2); /* 等待服务器启动 */
    
    printf("Starting client tests...\n");
    
    /* TCP客户端测试 */
    nv_tcp_client_t tcp_client;
    if (nv_tcp_client_create(&tcp_client, &main_loop) == 0) {
        if (nv_tcp_client_connect(&tcp_client, "127.0.0.1", 8888) == 0) {
            printf("TCP client connected\n");
            
            /* 发送数据 */
            const char *msg = "Hello TCP Server!";
            nv_tcp_write(&tcp_client.tcp, msg, strlen(msg));
            
            /* 接收响应 */
            char buffer[1024];
            int bytes_read = nv_tcp_read(&tcp_client.tcp, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("TCP client received: %s\n", buffer);
            }
            
            sleep(1);
            nv_tcp_client_disconnect(&tcp_client);
        }
    }
    
    /* UDP客户端测试 */
    nv_udp_client_t udp_client;
    if (nv_udp_client_create(&udp_client, &main_loop) == 0) {
        if (nv_udp_client_connect(&udp_client, "127.0.0.1", 8889) == 0) {
            printf("UDP client connected\n");
            
            /* 发送数据 */
            const char *msg = "Hello UDP Server!";
            nv_udp_send(&udp_client.udp, msg, strlen(msg));
            
            /* 接收响应 */
            char buffer[1024];
            struct sockaddr_in server_addr;
            socklen_t addr_len = sizeof(server_addr);
            
            int bytes_recv = recvfrom(udp_client.udp.fd, buffer, sizeof(buffer) - 1, 0,
                                     (struct sockaddr*)&server_addr, &addr_len);
            if (bytes_recv > 0) {
                buffer[bytes_recv] = '\0';
                printf("UDP client received: %s\n", buffer);
            }
            
            nv_udp_client_disconnect(&udp_client);
        }
    }
    
    printf("Client tests completed\n");
    return NULL;
}

/* 主函数 */
int main() {
    printf("NV Network Demo\n");
    printf("===============\n");
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* 初始化事件循环 */
    nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
    config.max_events = 256;
    config.timeout_ms = 100;
    config.enable_timers = 1;
    config.enable_idle = 1;
    
    if (nv_loop_init(&main_loop, &config) != 0) {
        perror("Failed to initialize event loop");
        return EXIT_FAILURE;
    }
    
    printf("Event loop initialized successfully\n");
    
    /* 创建TCP服务器 */
    if (nv_tcp_server_create(&tcp_server, &main_loop, 8888) != 0) {
        perror("Failed to create TCP server");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    if (nv_tcp_server_start(&tcp_server) != 0) {
        perror("Failed to start TCP server");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("TCP server started on port 8888\n");
    
    /* 创建UDP服务器 */
    if (nv_udp_server_create(&udp_server, &main_loop, 8889) != 0) {
        perror("Failed to create UDP server");
        nv_tcp_server_stop(&tcp_server);
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    if (nv_udp_server_start(&udp_server) != 0) {
        perror("Failed to start UDP server");
        nv_tcp_server_stop(&tcp_server);
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("UDP server started on port 8889\n");
    
    /* 创建客户端测试线程 */
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_test_thread, NULL) != 0) {
        perror("Failed to create client test thread");
    }
    
    /* 运行事件循环 */
    printf("Starting event loop...\n");
    printf("Press Ctrl+C to stop\n");
    
    if (nv_loop_run(&main_loop) != 0) {
        perror("Event loop run failed");
    }
    
    printf("Event loop stopped\n");
    
    /* 等待客户端测试线程完成 */
    pthread_join(client_thread, NULL);
    
    /* 清理资源 */
    nv_tcp_server_stop(&tcp_server);
    nv_udp_server_stop(&udp_server);
    nv_loop_cleanup(&main_loop);
    
    printf("Cleanup completed\n");
    
    return EXIT_SUCCESS;
}
