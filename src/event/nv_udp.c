
#include "nv_udp.h"
#include <nv_socket.h>
#include <nv_mem.h>
int nv_udp_init(nv_loop_t* loop, nv_udp_t* udp){
    udp->socketfd = nv_udp_socket_create();
    if(udp->socketfd < 0){
        return -1;
    }
    return 0; 
}
int nv_udp_bind(nv_udp_t* udp,uint32_t port){
    if(nv_socket_bind(udp->socketfd, port) < 0){
        nv_socket_close(udp->socketfd);
        return -1;
    }
     return 0; 
}
int nv_udp_listen(nv_udp_t* udp, nv_int32 backlog){
    /*if(nv_udp_socket_listen(udp->socketfd, backlog) < 0){
        nv_socket_close(udp->socketfd);
        return -1;
    }
    */
     return 0; 
}


nv_udp_t* nv_udp_accept(nv_udp_t* udp){
   nv_udp_t *client = nv_malloc(sizeof(nv_udp_t));
    
  //  client->socketfd = nv_udp_socket_accept(udp->socketfd);  

    return client;
    
}
int nv_udp_connect(nv_udp_t* udp, const nv_char* ip, nv_int32 port){
    
     return 0; 
}
int nv_udp_read(nv_udp_t* udp,nv_char *buff,nv_int32 size){
    
     return 0; 
}
int nv_udp_write(nv_udp_t* udp,nv_char *buff,nv_int32 size){
    
     return 0; 
}

int nv_udp_close(nv_udp_t* udp){
    
     return 0; 
}