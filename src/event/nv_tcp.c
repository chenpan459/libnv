#include "nv_tcp.h"
#include <nv_socket.h>
#include <nv_mem.h>


int nv_tcp_init(nv_loop_t* loop, nv_tcp_t* tcp){
    tcp->socketfd = nv_tcp_socket_create();
    if(tcp->socketfd < 0){
        return -1;
    }
    return 0;
}

int nv_tcp_bind(nv_tcp_t* tcp,uint32_t port){

    if(nv_socket_bind(tcp->socketfd, port) < 0){
        nv_socket_close(tcp->socketfd);
        return -1;
    }
       
    return 0;
}
int nv_tcp_listen(nv_tcp_t* tcp, nv_int32 backlog){
    
    if(nv_tcp_socket_listen(tcp->socketfd, backlog) < 0){
        nv_socket_close(tcp->socketfd);
        return -1;
    }
    
    return 0;
}
nv_tcp_t* nv_tcp_accept(nv_tcp_t* tcp){

    nv_tcp_t *client = nv_malloc(sizeof(nv_tcp_t));
    
    client->socketfd = nv_tcp_socket_accept(tcp->socketfd);  

    return client;
}

int nv_tcp_connect(nv_tcp_t* tcp, const nv_char* ip, nv_int32 port){
    
    if(nv_tcp_socket_connect(tcp->socketfd, ip, port) < 0){
        nv_socket_close(tcp->socketfd);
        return -1;
    }
    
    return 0;
}

int nv_tcp_read(nv_tcp_t* tcp,nv_char *buff,nv_int32 size){
    
    return nv_socket_recv(tcp->socketfd,buff,size,0);
}

int nv_tcp_write(nv_tcp_t* tcp,nv_char *buff,nv_int32 size){
    
    return nv_socket_send(tcp->socketfd,buff,size,0);
}

int nv_tcp_close(nv_tcp_t* tcp){
    
    nv_socket_close(tcp->socketfd);
    
    return 0;
}