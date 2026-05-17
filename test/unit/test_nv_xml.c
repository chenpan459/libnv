#include <nv_xml.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    const char *sample =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<config ver=\"1\">\n"
        "  <name>libnv</name>\n"
        "  <port>8080</port>\n"
        "</config>\n";
    const char *path = "/tmp/libnv_test_nv_xml.xml";
    nv_xml_t *root;
    nv_xml_t *cfg;
    nv_xml_t *name_node;
    nv_xml_t *port_node;
    const char *name;
    const char *ver;
    int port;
    char *xml_str;

    root = nv_xml_parse(sample);
    check(root != NULL, "parse sample");

    cfg = nv_xml_find_element(root, root, "config", NULL, NULL, 1);
    check(cfg != NULL, "find config");
    check(nv_xml_attr_get(cfg, "ver") != NULL && strcmp(nv_xml_attr_get(cfg, "ver"), "1") == 0, "config ver attr");

    name_node = nv_xml_find_path(cfg, "name");
    check(name_node != NULL, "find name path");
    check(nv_xml_get_text(name_node, &name) == NV_OK && strcmp(name, "libnv") == 0, "name text");

    port_node = nv_xml_find_element(cfg, cfg, "port", NULL, NULL, 0);
    check(port_node != NULL, "find port");
    check(nv_xml_get_text(port_node, &name) == NV_OK, "port text node");
    port = atoi(name);
    check(port == 8080, "port value");

    check(nv_xml_attr_set(cfg, "ver", "2") == NV_OK, "set attr");
    check(strcmp(nv_xml_attr_get(cfg, "ver"), "2") == 0, "attr updated");

    check(nv_xml_set_text(name_node, "libnv2") == NV_OK, "set name text");
    check(nv_xml_get_text(name_node, &name) == NV_OK && strcmp(name, "libnv2") == 0, "name updated");

    check(nv_xml_save_file(root, path) == NV_OK, "save file");

    nv_xml_free(root);
    root = nv_xml_load_file(path);
    check(root != NULL, "reload file");

    cfg = nv_xml_find_element(root, root, "config", NULL, NULL, 1);
    ver = nv_xml_attr_get(cfg, "ver");
    check(ver != NULL && strcmp(ver, "2") == 0, "reloaded ver attr");

    name_node = nv_xml_find_path(cfg, "name");
    check(nv_xml_get_text(name_node, &name) == NV_OK && strcmp(name, "libnv2") == 0, "reloaded name");

    xml_str = nv_xml_print(root);
    check(xml_str != NULL && strstr(xml_str, "libnv2") != NULL, "print contains update");
    nv_xml_string_free(xml_str);
    nv_xml_free(root);
    unlink(path);

    root = nv_xml_new_document("1.0");
    cfg = nv_xml_add_element(root, "root");
    check(cfg != NULL, "new doc element");
    check(nv_xml_add_text(cfg, "hello") != NULL, "add text");
    check(nv_xml_save_file(root, path) == NV_OK, "save new doc");
    nv_xml_free(root);

    check(nv_xml_parse("not xml") == NULL, "invalid xml");
    check(nv_xml_error_ptr() != NULL, "error ptr set");

    unlink(path);

    if (g_fail) {
        fprintf(stderr, "%d test(s) failed\n", g_fail);
        return 1;
    }
    printf("test_nv_xml: ok\n");
    return 0;
}
