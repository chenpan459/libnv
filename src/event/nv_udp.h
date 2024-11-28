#ifndef _NV_EVENT_UDP_H_INCLUDED_
#define _NV_EVENT_UDP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv.h>

int nv_udp_init(nv_loop_t* loop, nv_udp_t* udp);
int nv_udp_bind(nv_udp_t* udp,uint32_t port);
int nv_udp_listen(nv_udp_t* udp, nv_int32 backlog);
nv_udp_t* nv_udp_accept(nv_udp_t* udp);
int nv_udp_connect(nv_udp_t* udp, const nv_char* ip, nv_int32 port);
int nv_udp_read(nv_udp_t* udp,nv_char *buff,nv_int32 size);
int nv_udp_write(nv_udp_t* udp,nv_char *buff,nv_int32 size);
int nv_udp_close(nv_udp_t* udp);





#ifdef __cplusplus
}
#endif

#endif