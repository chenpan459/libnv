#ifndef _NV_INI_H_INCLUDED_
#define _NV_INI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <stdint.h>

typedef struct nv_ini_s nv_ini_t;

/* 从文件加载 INI 配置（支持 [section]、key=value、; 注释） */
nv_ini_t *nv_ini_load(const char *path);

/* 释放配置 */
void nv_ini_free(nv_ini_t *ini);

/* 读取配置项（section 可为 NULL 或 "" 表示全局段） */
const char *nv_ini_get_string(nv_ini_t *ini, const char *section,
                              const char *key, const char *default_val);
int  nv_ini_get_bool(nv_ini_t *ini, const char *section,
                     const char *key, int default_val);
int  nv_ini_get_int(nv_ini_t *ini, const char *section,
                    const char *key, int default_val);
int64_t nv_ini_get_int64(nv_ini_t *ini, const char *section,
                         const char *key, int64_t default_val);
double nv_ini_get_double(nv_ini_t *ini, const char *section,
                         const char *key, double default_val);

/* 是否存在指定键 */
int nv_ini_has_key(nv_ini_t *ini, const char *section, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* _NV_INI_H_INCLUDED_ */
