#include "nv_xml.h"

#include "mxml.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_nv_xml_error[512];

/* mxml API 无 const 参数，只读/序列化时安全去掉 const */
static mxml_node_t *nv_xml_unconst(const nv_xml_t *node)
{
    return (mxml_node_t *)(const void *)node;
}

static void nv_xml_error_cb(void *cbdata, const char *message)
{
    (void)cbdata;
    if (message == NULL) {
        g_nv_xml_error[0] = '\0';
        return;
    }
    snprintf(g_nv_xml_error, sizeof(g_nv_xml_error), "%s", message);
}

static mxml_options_t *nv_xml_options(void)
{
    static mxml_options_t *opts = NULL;

    if (opts == NULL) {
        opts = mxmlOptionsNew();
        if (opts != NULL) {
            mxmlOptionsSetErrorCallback(opts, nv_xml_error_cb, NULL);
            mxmlOptionsSetWrapMargin(opts, 0);
        }
    }
    return opts;
}

const char *nv_xml_error_ptr(void)
{
    return (g_nv_xml_error[0] != '\0') ? g_nv_xml_error : NULL;
}

nv_xml_t *nv_xml_parse(const char *text)
{
    mxml_options_t *opts;

    g_nv_xml_error[0] = '\0';
    if (text == NULL) {
        return NULL;
    }
    opts = nv_xml_options();
    if (opts == NULL) {
        return NULL;
    }
    return mxmlLoadString(NULL, opts, text);
}

nv_xml_t *nv_xml_load_file(const char *path)
{
    mxml_options_t *opts;

    g_nv_xml_error[0] = '\0';
    if (path == NULL) {
        return NULL;
    }
    opts = nv_xml_options();
    if (opts == NULL) {
        return NULL;
    }
    return mxmlLoadFilename(NULL, opts, path);
}

void nv_xml_free(nv_xml_t *root)
{
    if (root != NULL) {
        mxmlDelete(root);
    }
}

int nv_xml_save_file(const nv_xml_t *root, const char *path)
{
    mxml_options_t *opts;

    if (root == NULL || path == NULL) {
        return NV_ERROR;
    }
    opts = nv_xml_options();
    if (opts == NULL) {
        return NV_ERROR;
    }
    g_nv_xml_error[0] = '\0';
    return mxmlSaveFilename(nv_xml_unconst(root), opts, path) ? NV_OK : NV_ERROR;
}

char *nv_xml_print(const nv_xml_t *root)
{
    mxml_options_t *opts;

    if (root == NULL) {
        return NULL;
    }
    opts = nv_xml_options();
    if (opts == NULL) {
        return NULL;
    }
    return mxmlSaveAllocString(nv_xml_unconst(root), opts);
}

void nv_xml_string_free(char *str)
{
    if (str != NULL) {
        free(str);
    }
}

nv_xml_t *nv_xml_new_document(const char *version)
{
    if (version == NULL) {
        version = "1.0";
    }
    return mxmlNewXML(version);
}

nv_xml_t *nv_xml_add_element(nv_xml_t *parent, const char *name)
{
    if (parent == NULL || name == NULL) {
        return NULL;
    }
    return mxmlNewElement(parent, name);
}

nv_xml_t *nv_xml_add_text(nv_xml_t *parent, const char *text)
{
    if (parent == NULL || text == NULL) {
        return NULL;
    }
    return mxmlNewText(parent, 0, text);
}

nv_xml_t *nv_xml_add_integer(nv_xml_t *parent, long value)
{
    if (parent == NULL) {
        return NULL;
    }
    return mxmlNewInteger(parent, value);
}

nv_xml_t *nv_xml_find_element(nv_xml_t *node, nv_xml_t *top,
                              const char *name, const char *attr,
                              const char *value, int descend_all)
{
    mxml_descend_t descend = descend_all ? MXML_DESCEND_ALL : MXML_DESCEND_FIRST;

    if (node == NULL) {
        return NULL;
    }
    return mxmlFindElement(node, top, name, attr, value, descend);
}

nv_xml_t *nv_xml_find_path(nv_xml_t *node, const char *path)
{
    if (node == NULL || path == NULL) {
        return NULL;
    }
    return mxmlFindPath(node, path);
}

nv_xml_t *nv_xml_first_child(const nv_xml_t *node)
{
    if (node == NULL) {
        return NULL;
    }
    return mxmlGetFirstChild(nv_xml_unconst(node));
}

nv_xml_t *nv_xml_next_sibling(const nv_xml_t *node)
{
    if (node == NULL) {
        return NULL;
    }
    return mxmlGetNextSibling(nv_xml_unconst(node));
}

const char *nv_xml_element_name(const nv_xml_t *node)
{
    mxml_node_t *n;

    if (node == NULL) {
        return NULL;
    }
    n = nv_xml_unconst(node);
    if (mxmlGetType(n) != MXML_TYPE_ELEMENT) {
        return NULL;
    }
    return mxmlGetElement(n);
}

const char *nv_xml_attr_get(const nv_xml_t *elem, const char *name)
{
    mxml_node_t *n;

    if (elem == NULL || name == NULL) {
        return NULL;
    }
    n = nv_xml_unconst(elem);
    if (mxmlGetType(n) != MXML_TYPE_ELEMENT) {
        return NULL;
    }
    return mxmlElementGetAttr(n, name);
}

int nv_xml_attr_set(nv_xml_t *elem, const char *name, const char *value)
{
    if (elem == NULL || name == NULL || mxmlGetType(elem) != MXML_TYPE_ELEMENT) {
        return NV_ERROR;
    }
    mxmlElementSetAttr(elem, name, value != NULL ? value : "");
    return NV_OK;
}

int nv_xml_set_text(nv_xml_t *node, const char *text)
{
    if (node == NULL || text == NULL) {
        return NV_ERROR;
    }
    if (mxmlGetType(node) == MXML_TYPE_TEXT) {
        return mxmlSetText(node, 0, text) ? NV_OK : NV_ERROR;
    }
    if (mxmlGetType(node) == MXML_TYPE_ELEMENT) {
        nv_xml_t *child = mxmlGetFirstChild(node);
        while (child != NULL) {
            if (mxmlGetType(child) == MXML_TYPE_TEXT) {
                return mxmlSetText(child, 0, text) ? NV_OK : NV_ERROR;
            }
            child = mxmlGetNextSibling(child);
        }
        return nv_xml_add_text(node, text) != NULL ? NV_OK : NV_ERROR;
    }
    return NV_ERROR;
}

int nv_xml_set_element_name(nv_xml_t *elem, const char *name)
{
    if (elem == NULL || name == NULL || mxmlGetType(elem) != MXML_TYPE_ELEMENT) {
        return NV_ERROR;
    }
    return mxmlSetElement(elem, name) ? NV_OK : NV_ERROR;
}

int nv_xml_remove(nv_xml_t *node)
{
    if (node == NULL) {
        return NV_ERROR;
    }
    mxmlRemove(node);
    mxmlDelete(node);
    return NV_OK;
}

int nv_xml_get_text(const nv_xml_t *node, const char **out)
{
    bool ws = false;
    const char *text;

    if (out == NULL) {
        return NV_ERROR;
    }
    *out = NULL;
    mxml_node_t *n;

    if (node == NULL) {
        return NV_ERROR;
    }
    n = nv_xml_unconst(node);
    if (mxmlGetType(n) == MXML_TYPE_TEXT) {
        text = mxmlGetText(n, &ws);
        if (text == NULL) {
            return NV_ERROR;
        }
        *out = text;
        return NV_OK;
    }
    if (mxmlGetType(n) == MXML_TYPE_ELEMENT) {
        nv_xml_t *child = mxmlGetFirstChild(n);
        while (child != NULL) {
            if (mxmlGetType(child) == MXML_TYPE_TEXT) {
                text = mxmlGetText(child, &ws);
                if (text != NULL) {
                    *out = text;
                    return NV_OK;
                }
            }
            child = mxmlGetNextSibling(child);
        }
    }
    return NV_ERROR;
}

int nv_xml_get_int(const nv_xml_t *node, int *out)
{
    if (out == NULL || node == NULL) {
        return NV_ERROR;
    }
    if (mxmlGetType(nv_xml_unconst(node)) != MXML_TYPE_INTEGER) {
        return NV_ERROR;
    }
    *out = (int)mxmlGetInteger(nv_xml_unconst(node));
    return NV_OK;
}

int nv_xml_is_element(const nv_xml_t *node)
{
    return (node != NULL && mxmlGetType(nv_xml_unconst(node)) == MXML_TYPE_ELEMENT) ? 1 : 0;
}

int nv_xml_is_text(const nv_xml_t *node)
{
    return (node != NULL && mxmlGetType(nv_xml_unconst(node)) == MXML_TYPE_TEXT) ? 1 : 0;
}
