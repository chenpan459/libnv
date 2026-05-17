#include <nv_json.h>
#include <stdio.h>
#include <string.h>

static int g_fail;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        g_fail++;
    }
}

int main(void)
{
    const char *raw = "{\"name\":\"libnv\",\"ver\":1,\"ok\":true,\"tags\":[\"a\",\"b\"]}";
    nv_json_t *root;
    const char *name;
    int ver;
    int ok;
    char *out;
    nv_json_t *packed;
    nv_json_t *tags;
    nv_json_t *tag0;

    root = nv_json_parse(raw);
    check(root != NULL, "parse root");
    check(nv_json_get_string(root, "name", &name) == NV_OK, "get name");
    check(name != NULL && strcmp(name, "libnv") == 0, "name value");
    check(nv_json_get_int(root, "ver", &ver) == NV_OK && ver == 1, "get ver");
    check(nv_json_get_bool(root, "ok", &ok) == NV_OK && ok == 1, "get ok");

    tags = nv_json_object_get(root, "tags");
    check(nv_json_is_array(tags), "tags is array");
    check(nv_json_array_size(tags) == 2, "tags size");
    tag0 = nv_json_array_get(tags, 0);
    check(nv_json_is_string(tag0), "tag0 string");
    check(strcmp(nv_json_string_value(tag0), "a") == 0, "tag0 value");

    packed = nv_json_create_object();
    check(packed != NULL, "create object");
    check(nv_json_add_string(packed, "cmd", "ping") == NV_OK, "add string");
    check(nv_json_add_number(packed, "id", 42) == NV_OK, "add number");
    check(nv_json_add_bool(packed, "ack", 1) == NV_OK, "add bool");

    out = nv_json_print_unformatted(packed);
    check(out != NULL, "print packed");
    check(strstr(out, "\"cmd\":\"ping\"") != NULL, "packed cmd");
    check(strstr(out, "\"id\":42") != NULL, "packed id");

    nv_json_string_free(out);
    nv_json_free(packed);
    nv_json_free(root);

    check(nv_json_parse("{bad") == NULL, "invalid json");
    check(nv_json_error_ptr() != NULL, "error ptr on fail");

    if (g_fail) {
        fprintf(stderr, "%d test(s) failed\n", g_fail);
        return 1;
    }
    printf("test_nv_json: ok\n");
    return 0;
}
