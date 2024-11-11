
/************************************************
 * @文件名: nv_socket_types.h
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本数据类型
 ***********************************************/



#ifndef _NV_SOCKET_H_INCLUDED_
#define _NV_SOCKET_H_INCLUDED_

#include <nv_linux_config.h>

#include <stdint.h>

// 服务器参数结构体
typedef struct {
    int port;
    const char *message;
} nv_server_params_t;


// 创建并绑定套接字
int nv_socket_create(const nv_server_params_t *params);

// 监听并接受连接
int nv_socket_accept(int server_fd);

// 发送和接收数据
void nv_socket_send_receive(int socket, const char *message);

// 关闭套接字
void nv_socket_close(int socket);






#endif /* _NV_SOCKET_H_INCLUDED_ */
