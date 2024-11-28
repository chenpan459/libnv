#ifndef _NV_H_INCLUDED_
#define _NV_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include  <nv_config.h>

/* The abstract base class of all handles. */
struct nv_handle_s {
    
};
struct nv_tcp_s {
    struct nv_handle_s handle;
    int socketfd;
    int port;
    int family;
    int type;
    int protocol;
    int flags; 
};
struct nv_loop_s {
  
};





/* Handle types. */
typedef struct nv_handle_s nv_handle_t;
typedef struct nv_tcp_s    nv_tcp_t;
typedef struct nv_loop_s   nv_loop_t;

#ifdef __cplusplus
}
#endif

#endif