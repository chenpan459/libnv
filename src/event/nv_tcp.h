#ifndef _NV_EVENT_TCP_H_INCLUDED_
#define _NV_EVENT_TCP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv.h>




int nv_tcp_init(nv_loop_t* loop, nv_tcp_t* tcp);
int nv_tcp_bind(nv_tcp_t* tcp,nv_char * ip,uint32_t port);
int nv_tcp_listen(nv_tcp_t* tcp, nv_int32 backlog);
nv_tcp_t* nv_tcp_accept(nv_tcp_t* tcp, nv_tcp_t* client);
int nv_tcp_connect(nv_tcp_t* tcp, const nv_char* ip, nv_int32 port);
int nv_tcp_read(nv_tcp_t* tcp,nv_char *buff,nv_int32 size);
int nv_tcp_write(nv_tcp_t* tcp,nv_char *buff,nv_int32 size);
int nv_tcp_close(nv_tcp_t* tcp);



#ifdef __cplusplus
}
#endif

#endif