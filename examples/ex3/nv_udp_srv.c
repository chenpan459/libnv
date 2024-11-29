#include <nv.h>
#include <nv_udp.h>
#include <nv_log.h>
#include <nv_version.h>
#include <nv_socket.h>
#include <nv_string.h>
#include <nv_mem.h>
#include <nv_sys.h>
#include <nv_thread.h>
#include <nv_udp.h>


/*************************************
 * 
1. 查找占用端口的进程

您可以使用以下命令来查找占用特定端口的进程：

    netstat -tulpn | grep <PORT_NUMBER>：列出所有使用指定端口的进程。
    lsof -i :<PORT_NUMBER>：显示使用指定端口的进程信息。

2. 终止占用端口的进程

一旦找到占用端口的进程，您可以使用以下命令来终止该进程：

    kill -9 <PID>：强制终止指定PID的进程。
    killall <PROCESS_NAME>：终止所有名为<PROCESS_NAME>的进程。

3. 使用fuser命令释放端口

如果您不想直接终止进程，可以使用fuser命令尝试释放端口：

    fuser -k <PORT_NUMBER>/udp：强制释放指定的udp端口。


 打开另一个终端窗口，使用 nc 命令连接到 udp 服务器：
 nc 127.0.0.1 60000

*********************/


int main() {
   
    nv_loop_t loop;
    nv_udp_t  udp;
    nv_int32  ret;
    char buff[1024];
    int len=0;
 
    nv_log_debug("compile_version: %s\n",hv_compile_version());
    
    ret = nv_udp_init(&loop,&udp); 
    if(ret < 0){
        nv_log_debug("nv_udp_init failed\n");
        return -1;
    }

    ret = nv_udp_bind(&udp,8080);
    if(ret < 0){
        nv_log_debug("nv_udp_connect failed\n");
        return -1;
    }
    while(1){

      
        len = nv_udp_read(&udp,buff,1024);
        nv_assert_pointer(len>0,len);
        nv_log_debug("recv:%s\n",buff);
        nv_memset(buff,0,1024);
        len = nv_udp_sendto(&udp,"hello",5,&udp.src_addr);
        nv_assert_pointer(len>0,len);
        nv_sleep(1);
 
    }
    
    nv_udp_close(&udp);
    
    return 0;
}



