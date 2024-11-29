#ifndef _NV_EVENT_UDP_H_INCLUDED_
#define _NV_EVENT_UDP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv.h>

int nv_udp_init(nv_loop_t* loop, nv_udp_t* udp);
int nv_udp_bind(nv_udp_t* udp,uint32_t port);

int nv_udp_read(nv_udp_t* udp,nv_char *buff,nv_int32 size);
int nv_udp_write(nv_udp_t* udp,nv_char *buff,nv_int32 size,nv_char *ip,int port);
int nv_udp_sendto(nv_udp_t* udp,nv_char *buff,nv_int32 size,struct sockaddr_in *dest_addr);
int nv_udp_close(nv_udp_t* udp);





#ifdef __cplusplus
}
#endif

#endif