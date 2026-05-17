#ifndef _NV_XML_H_INCLUDED_
#define _NV_XML_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_base.h>
#include <stddef.h>

struct _mxml_node_s;
typedef struct _mxml_node_s nv_xml_t;

/* ---------- 解析 / 释放 ---------- */

/** 从 UTF-8 字符串解析 XML，失败返回 NULL */
nv_xml_t *nv_xml_parse(const char *text);

/** 从文件加载 XML */
nv_xml_t *nv_xml_load_file(const char *path);

/** 释放整棵 XML 树（含子节点） */
void nv_xml_free(nv_xml_t *root);

/** 最近一次 load/parse 失败时的错误信息 */
const char *nv_xml_error_ptr(void);

/* ---------- 保存 / 序列化 ---------- */

/** 保存到文件（覆盖写入） */
int nv_xml_save_file(const nv_xml_t *root, const char *path);

/** 分配格式化的 XML 字符串，用 nv_xml_string_free 释放 */
char *nv_xml_print(const nv_xml_t *root);

void nv_xml_string_free(char *str);

/* ---------- 创建 ---------- */

/** 新建 XML 文档根（含 <?xml version="1.0"?>） */
nv_xml_t *nv_xml_new_document(const char *version);

nv_xml_t *nv_xml_add_element(nv_xml_t *parent, const char *name);
nv_xml_t *nv_xml_add_text(nv_xml_t *parent, const char *text);
nv_xml_t *nv_xml_add_integer(nv_xml_t *parent, long value);

/* ---------- 查找 ---------- */

nv_xml_t *nv_xml_find_element(nv_xml_t *node, nv_xml_t *top,
                              const char *name, const char *attr,
                              const char *value, int descend_all);

nv_xml_t *nv_xml_find_path(nv_xml_t *node, const char *path);

nv_xml_t *nv_xml_first_child(const nv_xml_t *node);
nv_xml_t *nv_xml_next_sibling(const nv_xml_t *node);
const char *nv_xml_element_name(const nv_xml_t *node);

/* ---------- 属性 ---------- */

const char *nv_xml_attr_get(const nv_xml_t *elem, const char *name);
int nv_xml_attr_set(nv_xml_t *elem, const char *name, const char *value);

/* ---------- 修改节点内容 ---------- */

int nv_xml_set_text(nv_xml_t *node, const char *text);
int nv_xml_set_element_name(nv_xml_t *elem, const char *name);
int nv_xml_remove(nv_xml_t *node);

/* ---------- 读取 ---------- */

int nv_xml_get_text(const nv_xml_t *node, const char **out);
int nv_xml_get_int(const nv_xml_t *node, int *out);
int nv_xml_is_element(const nv_xml_t *node);
int nv_xml_is_text(const nv_xml_t *node);

#ifdef __cplusplus
}
#endif

#endif
