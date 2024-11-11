/************************************************
 * @文件名: nv_socket.c
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 ***********************************************/

#include <nv_socket.h>

typedef int  socket_fd;


/* Open a socket in non-blocking close-on-exec mode, atomically if possible. */
int nv_socket_create(int domain, int type, int protocol) {
  int sockfd;
  int err;

#if defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
  sockfd = socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
  if (sockfd != -1)
    return sockfd;

  if (errno != EINVAL)
    return NV__ERR(errno);
#endif

  sockfd = socket(domain, type, protocol);
  if (sockfd == -1)
    return NV__ERR(errno);

  err = uv__nonblock(sockfd, 1);
  if (err == 0)
    err = uv__cloexec(sockfd, 1);

  if (err) {
    uv__close(sockfd);
    return err;
  }

#if defined(SO_NOSIGPIPE)
  {
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
  }
#endif

  return sockfd;
}

int nv_socket_bind(socket_fd fd)
{
   return 0;
}

int nv_socket_listen(socket_fd fd)
{
   return 0;
}


int nv_socket_accept(socket_fd fd)
{
   return 0;
}

int nv_socket_connet(socket_fd fd)
{
   return 0;
}


int nv_socket_rcv(socket_fd fd)
{
   return 0;
}

int nv_socket_send(socket_fd fd)
{
   return 0;
}


int nv_socket_close(socket_fd fd)
{
   return  close(socket);
}






