


#ifndef _NV_CONFIG_H_INCLUDED_
#define _NV_CONFIG_H_INCLUDED_


//#include <ngx_auto_headers.h>
#include <stdint.h>




#if defined __DragonFly__ && !defined __FreeBSD__
#define __FreeBSD__        4
#define __FreeBSD_version  480101
#endif


#if (NGX_FREEBSD)
#include <ngx_freebsd_config.h>


#elif (NGX_LINUX)
#include <ngx_linux_config.h>

#endif


#ifndef NGX_HAVE_SO_SNDLOWAT
#define NGX_HAVE_SO_SNDLOWAT     1
#endif



#define nv_signal_helper(n)     SIG##n
#define nv_signal_value(n)      nv_signal_helper(n)

#define nv_random               random
#define nv_cdecl
#define nv_libc_cdecl


typedef intptr_t        nv_int_t;
typedef uintptr_t       nv_uint_t;
typedef intptr_t        nv_flag_t;


#define NV_INT32_LEN   (sizeof("-2147483648") - 1)
#define NV_INT64_LEN   (sizeof("-9223372036854775808") - 1)

#if (NGX_PTR_SIZE == 4)
#define NV_INT_T_LEN   NGX_INT32_LEN
#define NV_MAX_INT_T_VALUE  2147483647

#else
#define NV_INT_T_LEN   NGX_INT64_LEN
#define NV_MAX_INT_T_VALUE  9223372036854775807
#endif


#ifndef NV_ALIGNMENT
#define NV_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif

#define nv_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define nv_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


#define nv_abort       abort


/* TODO: platform specific: array[NGX_INVALID_ARRAY_INDEX] must cause SIGSEGV */
#define NV_INVALID_ARRAY_INDEX 0x80000000


/* TODO: auto_conf: ngx_inline   inline __inline __inline__ */
#ifndef nv_inline
#define nv_inline      inline
#endif

#ifndef INADDR_NONE  /* Solaris */
#define INADDR_NONE  ((unsigned int) -1)
#endif


#define NV_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define NV_MAX_INT32_VALUE   (uint32_t) 0x7fffffff




#endif /* _NGX_CONFIG_H_INCLUDED_ */
