#include <nv.h>
#include <nv_udp.h>
#include <nv_log.h>
#include <nv_version.h>
#include <nv_socket.h>
#include <nv_string.h>
#include <nv_mem.h>


int main() {

    nv_loop_t loop;
    nv_udp_t  udp;
    nv_int32  ret;
    char buff[1024];
    nv_log_debug("compile_version: %s\n",hv_compile_version());
    
    ret = nv_udp_init(&loop,&udp); 
    if(ret < 0){
        nv_log_debug("nv_udp_init failed\n");
        return -1;
    }
    ret = nv_udp_connect(&udp,"172.21.17.184",60000);
    if(ret < 0){
        nv_log_debug("nv_udp_connect failed\n");
        return -1;
    }

    while(1){
          nv_udp_write(&udp,"hello",5);
          sleep(1);
          nv_udp_read(&udp,buff,1024);
          nv_log_debug("recv:%s\n",buff);
          nv_memset(buff,0,1024);
    }
    
    nv_udp_close(&udp);
    

    return 0;

    
}



