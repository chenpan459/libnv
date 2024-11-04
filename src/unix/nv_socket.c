
#include <nv_socket.h>


/* Open a socket in non-blocking close-on-exec mode, atomically if possible. */
int uv_socket_create(int domain, int type, int protocol) {
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

int nv_socket_bind()
{
   return 0;
}

int nv_socket_listen()
{
   return 0;
}


int nv_socket_accept()
{
   return 0;
}


int nv_socket_read()
{
   return 0;
}

int nv_socket_write()
{
   return 0;
}





