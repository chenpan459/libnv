#include <nv_socket.h>
#include <nv_string.h>
#include <nv_mem.h>
#include <nv_sys.h>
#include <nv_thread.h>

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

    fuser -k <PORT_NUMBER>/tcp：强制释放指定的TCP端口。


 打开另一个终端窗口，使用 nc 命令连接到 TCP 服务器：
 nc 127.0.0.1 60000

*********************/
void *client_thread(void *arg){
    struct nv_tcp_s *client = (struct nv_tcp_s*)arg;
    char buff[1024];
    int len=0;
    nv_log_debug("client_thread enter\n");
    if(client == NULL){
        return NULL;
    }

    while(1){
        len = nv_tcp_read(client,buff,1024);
        nv_assert_pointer(len>0,len);
        nv_log_debug("recv:%s\n",buff);
        nv_memset(buff,0,1024);
        len = nv_tcp_write(client,"hello",5);
        nv_assert_pointer(len>0,len);
        nv_sleep(1);
    }
    nv_tcp_close(client);
    nv_free(client);
    nv_log_debug("client_thread exit\n");
    return NULL;
}

int main() {
   
    struct nv_loop_s loop;
    struct nv_tcp_s  tcp;
    nv_int32  ret;
    struct nv_tcp_s *client;
 
    nv_log_debug("compile_version: %s\n",hv_compile_version());
    
    ret = nv_tcp_init(&loop,&tcp); 
    if(ret < 0){
        nv_log_debug("nv_tcp_init failed\n");
        return -1;
    }

    ret = nv_tcp_bind(&tcp,8080);
    if(ret < 0){
        nv_log_debug("nv_tcp_connect failed\n");
        return -1;
    }
    ret = nv_tcp_listen(&tcp,50);
    nv_assert(ret== NV_SUCC,ret);

    while(1){

          nv_log_debug("waiting for connect\n");
          client = nv_tcp_accept(&tcp);
          nv_assert(client != NULL,NV_FAIL);
          nv_pthread_t tid;
          nv_create_thread(&tid,NULL,client_thread,(void*)client);
    }
    
    nv_tcp_close(&tcp);
    
    return 0;
}



