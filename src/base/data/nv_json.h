#ifndef _NV_JSON_H_INCLUDED_
#define _NV_JSON_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_base.h>
#include <stddef.h>

struct cJSON;

/** JSON 树节点，底层为 cJSON */
typedef struct cJSON nv_json_t;

/* ---------- 解析 ---------- */

/** 解析 UTF-8 JSON 文本，失败返回 NULL */
nv_json_t *nv_json_parse(const char *text);

/** 解析指定长度的 JSON 缓冲区 */
nv_json_t *nv_json_parse_len(const char *text, size_t len);

/** 释放 parse/create 得到的整棵 JSON 树 */
void nv_json_free(nv_json_t *root);

/** 最近一次解析失败时的错误位置（成功时为 NULL） */
const char *nv_json_error_ptr(void);

/* ---------- 序列化（封装输出） ---------- */

/** 格式化的 JSON 字符串，调用方用 nv_json_string_free 释放 */
char *nv_json_print(const nv_json_t *root);

/** 紧凑 JSON 字符串 */
char *nv_json_print_unformatted(const nv_json_t *root);

void nv_json_string_free(char *str);

/* ---------- 创建 ---------- */

nv_json_t *nv_json_create_null(void);
nv_json_t *nv_json_create_bool(int boolean);
nv_json_t *nv_json_create_number(double num);
nv_json_t *nv_json_create_string(const char *string);
nv_json_t *nv_json_create_array(void);
nv_json_t *nv_json_create_object(void);

/* ---------- 对象 / 数组 组装 ---------- */

int nv_json_array_add(nv_json_t *array, nv_json_t *item);
int nv_json_object_add(nv_json_t *object, const char *name, nv_json_t *item);

int nv_json_add_null(nv_json_t *object, const char *name);
int nv_json_add_bool(nv_json_t *object, const char *name, int boolean);
int nv_json_add_number(nv_json_t *object, const char *name, double number);
int nv_json_add_string(nv_json_t *object, const char *name, const char *string);
int nv_json_add_object(nv_json_t *object, const char *name);
int nv_json_add_array(nv_json_t *object, const char *name);

/* ---------- 读取 ---------- */

nv_json_t *nv_json_object_get(const nv_json_t *object, const char *name);
int nv_json_array_size(const nv_json_t *array);
nv_json_t *nv_json_array_get(const nv_json_t *array, int index);

int nv_json_get_string(const nv_json_t *object, const char *name, const char **out);
int nv_json_get_int(const nv_json_t *object, const char *name, int *out);
int nv_json_get_double(const nv_json_t *object, const char *name, double *out);
int nv_json_get_bool(const nv_json_t *object, const char *name, int *out);

int nv_json_is_null(const nv_json_t *item);
int nv_json_is_bool(const nv_json_t *item);
int nv_json_is_number(const nv_json_t *item);
int nv_json_is_string(const nv_json_t *item);
int nv_json_is_array(const nv_json_t *item);
int nv_json_is_object(const nv_json_t *item);

/** 取任意节点上的字符串值（非 string 类型返回 NULL） */
const char *nv_json_string_value(const nv_json_t *item);

#ifdef __cplusplus
}
#endif

#endif
