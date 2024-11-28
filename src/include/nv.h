#ifndef _NV_H_INCLUDED_
#define _NV_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>

/* 状态返回值 */
#define NV_SUCC 0        /* 成功 */
#define NV_FAIL -1       /* 失败 */
#define NV_EINVAL -2     /* 无效参数 */
#define NV_ENOMEM -3     /* 内存不足 */
#define NV_EIO -4        /* I/O 错误 */
#define NV_EAGAIN -5     /* 资源暂时不可用 */
#define NV_ECONNREFUSED -6 /* 连接被拒绝 */







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