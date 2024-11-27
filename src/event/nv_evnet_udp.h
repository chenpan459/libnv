#ifndef _NV_EVENT_UDP_H_INCLUDED_
#define _NV_EVENT_UDP_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>

int nv_udp_server(const char* ip, int port) ;
int nv_udp_client(const char* ip, int port) ;


#ifdef __cplusplus
}
#endif

#endif