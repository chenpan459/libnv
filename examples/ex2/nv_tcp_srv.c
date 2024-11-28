#include <nv.h>
#include <nv_tcp.h>
#include <nv_log.h>
#include <nv_version.h>
#include <nv_socket.h>
#include <nv_string.h>

int main() {

    nv_loop_t loop;
    nv_tcp_t  tcp;
    nv_int32  ret;
    nv_tcp_t* client;
    char buff[1024];
    nv_log_debug("compile_version: %s\n",hv_compile_version());
    
    ret = nv_tcp_init(&loop,&tcp); 
    if(ret < 0){
        nv_log_debug("nv_tcp_init failed\n");
        return -1;
    }


    ret = nv_tcp_bind(&tcp,"127.0.0.1",60000);
    if(ret < 0){
        nv_log_debug("nv_tcp_connect failed\n");
        return -1;
    }

    while(1){

          nv_log_debug("waiting for connect\n");
           nv_tcp_accept(&tcp,&client);
        //   if(ret < 0){
        //     nv_log_debug("nv_tcp_accept failed\n");
        //     return -1;
        //  }
         while(1){

            nv_tcp_read(client,buff,1024);
            nv_log_debug("recv:%s\n",buff);
            nv_memset(buff,0,1024);
            nv_tcp_write(client,"hello",5);
            sleep(1);
         } 
    }
    
    nv_tcp_close(&tcp);
    

    return 0;

    
}



