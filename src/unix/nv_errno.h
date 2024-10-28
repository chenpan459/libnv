
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NV_ERRNO_H_INCLUDED_
#define _NV_ERRNO_H_INCLUDED_

#include <sys/types.h> 

#include <nv_config.h>
#include <nv_core.h>


typedef int               nv_err_t;

#define NV_EPERM         EPERM
#define NV_ENOENT        ENOENT
#define NV_ENOPATH       ENOENT
#define NV_ESRCH         ESRCH
#define NV_EINTR         EINTR
#define NV_ECHILD        ECHILD
#define NV_ENOMEM        ENOMEM
#define NV_EACCES        EACCES
#define NV_EBUSY         EBUSY
#define NV_EEXIST        EEXIST
#define NV_EEXIST_FILE   EEXIST
#define NV_EXDEV         EXDEV
#define NV_ENOTDIR       ENOTDIR
#define NV_EISDIR        EISDIR
#define NV_EINVAL        EINVAL
#define NV_ENFILE        ENFILE
#define NV_EMFILE        EMFILE
#define NV_ENOSPC        ENOSPC
#define NV_EPIPE         EPIPE
#define NV_EINPROGRESS   EINPROGRESS
#define NV_ENOPROTOOPT   ENOPROTOOPT
#define NV_EOPNOTSUPP    EOPNOTSUPP
#define NV_EADDRINUSE    EADDRINUSE
#define NV_ECONNABORTED  ECONNABORTED
#define NV_ECONNRESET    ECONNRESET
#define NV_ENOTCONN      ENOTCONN
#define NV_ETIMEDOUT     ETIMEDOUT
#define NV_ECONNREFUSED  ECONNREFUSED
#define NV_ENAMETOOLONG  ENAMETOOLONG
#define NV_ENETDOWN      ENETDOWN
#define NV_ENETUNREACH   ENETUNREACH
#define NV_EHOSTDOWN     EHOSTDOWN
#define NV_EHOSTUNREACH  EHOSTUNREACH
#define NV_ENOSYS        ENOSYS
#define NV_ECANCELED     ECANCELED
#define NV_EILSEQ        EILSEQ
#define NV_ENOMOREFILES  0
#define NV_ELOOP         ELOOP
#define NV_EBADF         EBADF

#if (NGX_HAVE_OPENAT)
#define NV_EMLINK        EMLINK
#endif

#if (__hpux__)
#define NV_EAGAIN        EWOULDBLOCK
#else
#define NV_EAGAIN        EAGAIN
#endif


#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err


u_char *ngx_strerror(nv_err_t err, u_char *errstr, size_t size);
nv_int_t ngx_strerror_init(void);


#endif /* _NGX_ERRNO_H_INCLUDED_ */
