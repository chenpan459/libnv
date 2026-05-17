
#ifndef _NV_VERSION_H_INCLUDED_
#define _NV_VERSION_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>

/* 构建时由 CMake 生成（nv_build_info.h） */
#include <nv_build_info.h>

const char *nv_version_string(void);
const char *nv_build_time_string(void);

/* 兼容旧接口 */
const char *hv_compile_version(void);

#ifdef __cplusplus
}
#endif

#endif
