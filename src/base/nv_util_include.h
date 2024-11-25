#ifndef _NV_UTIL_INCLUDE_H_INCLUDED_
#define _NV_UTIL_INCLUDE_H_INCLUDED_



#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h> 
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>
#include <arpa/inet.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // 为了使用 inet_ntoa
#include <sys/select.h>
#include <sys/poll.h>
#include <termios.h>




#define NV_UTIL_TEST_ON 1

// 定义基本数据类型
typedef int8_t     nv_int8;
typedef uint8_t    nv_uint8;
typedef int16_t    nv_int16;
typedef uint16_t   nv_uint16;
typedef int32_t    nv_int32;
typedef uint32_t   nv_uint32;
typedef int64_t    nv_int64;
typedef uint64_t   nv_uint64;
typedef float      nv_float;
typedef double     nv_double;
typedef char       nv_char;
typedef unsigned char nv_uchar;
typedef void*       nv_pointer;


#endif