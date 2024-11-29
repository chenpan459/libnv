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


int nv_udp_read(nv_udp_t* udp,nv_char *buff,nv_int32 size){    
    //struct sockaddr_in src_addr;
    return nv_udp_socket_recvfrom(udp->socketfd, buff, size, &udp->src_addr);
}

int nv_udp_write(nv_udp_t* udp,nv_char *buff,nv_int32 size,nv_char *ip,int port){
    // 发送数据
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest_addr.sin_addr);
    return nv_udp_socket_sendto(udp->socketfd, buff, size, &dest_addr);
}

int nv_udp_sendto(nv_udp_t* udp,nv_char *buff,nv_int32 size,struct sockaddr_in *dest_addr){
    return nv_udp_socket_sendto(udp->socketfd, buff, size, dest_addr);
}

int nv_udp_close(nv_udp_t* udp){
    nv_socket_close(udp->socketfd);
     return 0; 
}