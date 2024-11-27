/************************************************
 * @文件名: nv_socket_types.h
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本数据类型
 ***********************************************/



#ifndef _NV_CORE_H_INCLUDED_
#define _NV_CORE_H_INCLUDED_



/*typedef struct nv_module_s          nv_module_t;
*/typedef struct nv_conf_s            nv_conf_t;
/*typedef struct nv_cycle_s           nv_cycle_t;
typedef struct nv_pool_s            nv_pool_t;
typedef struct nv_chain_s           nv_chain_t;
*/typedef struct nv_log_s             nv_log_t;
/*typedef struct nv_open_file_s       nv_open_file_t;
typedef struct nv_command_s         nv_command_t;
typedef struct nv_file_s            nv_file_t;
typedef struct nv_event_s           nv_event_t;
typedef struct nv_event_aio_s       nv_event_aio_t;
typedef struct nv_connection_s      nv_connection_t;
typedef struct nv_thread_task_s     nv_thread_task_t;
typedef struct nv_ssl_s             nv_ssl_t;
typedef struct nv_proxy_protocol_s  nv_proxy_protocol_t;
typedef struct nv_ssl_connection_s  nv_ssl_connection_t;
typedef struct nv_udp_connection_s  nv_udp_connection_t;

typedef void (*nv_event_handler_pt)(nv_event_t *ev);
typedef void (*nv_connection_handler_pt)(nv_connection_t *c);
*/


#define  NV_OK          0
#define  NV_ERROR      -1
#define  NV_AGAIN      -2
#define  NV_BUSY       -3
#define  NV_DONE       -4
#define  NV_DECLINED   -5
#define  NV_ABORT      -6

#include <stdio.h>
#include <stdarg.h>


/*#include <nv_atomic.h>
#include <nv_thread.h>
#include <nv_rbtree.h>
#include <nv_time.h>
#include <nv_socket.h>
#include <nv_string.h>
#include <nv_files.h>
#include <nv_shmem.h>
#include <nv_process.h>
#include <nv_user.h>
#include <nv_dlopen.h>
#include <nv_parse.h>
#include <nv_parse_time.h>
#include <nv_log.h>
#include <nv_alloc.h>
#include <nv_palloc.h>
#include <nv_buf.h>
#include <nv_queue.h>
#include <nv_array.h>
#include <nv_list.h>
#include <nv_hash.h>
#include <nv_file.h>
#include <nv_crc.h>
#include <nv_crc32.h>
#include <nv_murmurhash.h>
#include <nv_radix_tree.h>
#include <nv_times.h>
#include <nv_rwlock.h>
#include <nv_shmtx.h>
#include <nv_slab.h>
#include <nv_inet.h>
#include <nv_cycle.h>
#include <nv_resolver.h>

#include <nv_process_cycle.h>
#include <nv_conf_file.h>
#include <nv_module.h>
#include <nv_open_file_cache.h>
#include <nv_os.h>
#include <nv_connection.h>
#include <nv_syslog.h>
#include <nv_proxy_protocol.h>
*/


#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"


#define nv_abs(value)       (((value) >= 0) ? (value) : - (value))
#define nv_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define nv_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

void nv_cpuinfo(void);

#if (NV_HAVE_OPENAT)
#define NV_DISABLE_SYMLINKS_OFF        0
#define NV_DISABLE_SYMLINKS_ON         1
#define NV_DISABLE_SYMLINKS_NOTOWNER   2
#endif

#endif /* _NV_CORE_H_INCLUDED_ */
