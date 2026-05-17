#include "nv_json.h"

#include "cJSON.h"

#include <stdlib.h>

static int nv_json_check_ptr(const void *p)
{
    return p != NULL ? NV_OK : NV_ERROR;
}

nv_json_t *nv_json_parse(const char *text)
{
    if (text == NULL) {
        return NULL;
    }
    return cJSON_Parse(text);
}

nv_json_t *nv_json_parse_len(const char *text, size_t len)
{
    if (text == NULL || len == 0) {
        return NULL;
    }
    return cJSON_ParseWithLength(text, len);
}

void nv_json_free(nv_json_t *root)
{
    cJSON_Delete(root);
}

const char *nv_json_error_ptr(void)
{
    return cJSON_GetErrorPtr();
}

char *nv_json_print(const nv_json_t *root)
{
    if (root == NULL) {
        return NULL;
    }
    return cJSON_Print(root);
}

char *nv_json_print_unformatted(const nv_json_t *root)
{
    if (root == NULL) {
        return NULL;
    }
    return cJSON_PrintUnformatted(root);
}

void nv_json_string_free(char *str)
{
    if (str != NULL) {
        cJSON_free(str);
    }
}

nv_json_t *nv_json_create_null(void)
{
    return cJSON_CreateNull();
}

nv_json_t *nv_json_create_bool(int boolean)
{
    return cJSON_CreateBool(boolean ? 1 : 0);
}

nv_json_t *nv_json_create_number(double num)
{
    return cJSON_CreateNumber(num);
}

nv_json_t *nv_json_create_string(const char *string)
{
    if (string == NULL) {
        return NULL;
    }
    return cJSON_CreateString(string);
}

nv_json_t *nv_json_create_array(void)
{
    return cJSON_CreateArray();
}

nv_json_t *nv_json_create_object(void)
{
    return cJSON_CreateObject();
}

int nv_json_array_add(nv_json_t *array, nv_json_t *item)
{
    if (nv_json_check_ptr(array) != NV_OK || nv_json_check_ptr(item) != NV_OK) {
        return NV_ERROR;
    }
    if (!cJSON_IsArray(array)) {
        return NV_ERROR;
    }
    return cJSON_AddItemToArray(array, item) ? NV_OK : NV_ERROR;
}

int nv_json_object_add(nv_json_t *object, const char *name, nv_json_t *item)
{
    if (nv_json_check_ptr(object) != NV_OK || name == NULL || nv_json_check_ptr(item) != NV_OK) {
        return NV_ERROR;
    }
    if (!cJSON_IsObject(object)) {
        return NV_ERROR;
    }
    return cJSON_AddItemToObject(object, name, item) ? NV_OK : NV_ERROR;
}

int nv_json_add_null(nv_json_t *object, const char *name)
{
    nv_json_t *item = nv_json_create_null();
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_json_add_bool(nv_json_t *object, const char *name, int boolean)
{
    nv_json_t *item = nv_json_create_bool(boolean);
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_json_add_number(nv_json_t *object, const char *name, double number)
{
    nv_json_t *item = nv_json_create_number(number);
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_json_add_string(nv_json_t *object, const char *name, const char *string)
{
    nv_json_t *item = nv_json_create_string(string);
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_json_add_object(nv_json_t *object, const char *name)
{
    nv_json_t *item = nv_json_create_object();
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

int nv_json_add_array(nv_json_t *object, const char *name)
{
    nv_json_t *item = nv_json_create_array();
    if (item == NULL) {
        return NV_ERROR;
    }
    if (nv_json_object_add(object, name, item) != NV_OK) {
        nv_json_free(item);
        return NV_ERROR;
    }
    return NV_OK;
}

nv_json_t *nv_json_object_get(const nv_json_t *object, const char *name)
{
    if (object == NULL || name == NULL || !cJSON_IsObject(object)) {
        return NULL;
    }
    return cJSON_GetObjectItemCaseSensitive(object, name);
}

int nv_json_array_size(const nv_json_t *array)
{
    if (array == NULL || !cJSON_IsArray(array)) {
        return -1;
    }
    return cJSON_GetArraySize(array);
}

nv_json_t *nv_json_array_get(const nv_json_t *array, int index)
{
    if (array == NULL || !cJSON_IsArray(array)) {
        return NULL;
    }
    return cJSON_GetArrayItem(array, index);
}

int nv_json_get_string(const nv_json_t *object, const char *name, const char **out)
{
    nv_json_t *item;

    if (out == NULL) {
        return NV_ERROR;
    }
    *out = NULL;
    item = nv_json_object_get(object, name);
    if (item == NULL || !cJSON_IsString(item)) {
        return NV_ERROR;
    }
    *out = cJSON_GetStringValue(item);
    return (*out != NULL) ? NV_OK : NV_ERROR;
}

int nv_json_get_int(const nv_json_t *object, const char *name, int *out)
{
    nv_json_t *item;

    if (out == NULL) {
        return NV_ERROR;
    }
    item = nv_json_object_get(object, name);
    if (item == NULL || !cJSON_IsNumber(item)) {
        return NV_ERROR;
    }
    *out = (int)cJSON_GetNumberValue(item);
    return NV_OK;
}

int nv_json_get_double(const nv_json_t *object, const char *name, double *out)
{
    nv_json_t *item;

    if (out == NULL) {
        return NV_ERROR;
    }
    item = nv_json_object_get(object, name);
    if (item == NULL || !cJSON_IsNumber(item)) {
        return NV_ERROR;
    }
    *out = cJSON_GetNumberValue(item);
    return NV_OK;
}

int nv_json_get_bool(const nv_json_t *object, const char *name, int *out)
{
    nv_json_t *item;

    if (out == NULL) {
        return NV_ERROR;
    }
    item = nv_json_object_get(object, name);
    if (item == NULL || !cJSON_IsBool(item)) {
        return NV_ERROR;
    }
    *out = cJSON_IsTrue(item) ? 1 : 0;
    return NV_OK;
}

int nv_json_is_null(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsNull(item)) ? 1 : 0;
}

int nv_json_is_bool(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsBool(item)) ? 1 : 0;
}

int nv_json_is_number(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsNumber(item)) ? 1 : 0;
}

int nv_json_is_string(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsString(item)) ? 1 : 0;
}

int nv_json_is_array(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsArray(item)) ? 1 : 0;
}

int nv_json_is_object(const nv_json_t *item)
{
    return (item != NULL && cJSON_IsObject(item)) ? 1 : 0;
}

const char *nv_json_string_value(const nv_json_t *item)
{
    if (item == NULL || !cJSON_IsString(item)) {
        return NULL;
    }
    return cJSON_GetStringValue(item);
}
