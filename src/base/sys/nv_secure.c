#include "nv_secure.h"

#include <string.h>

int nv_secure_mem_equal(const void *a, const void *b, size_t len)
{
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    unsigned char        diff = 0;
    size_t               i;

    if (!pa || !pb) {
        return (pa == pb) ? 1 : 0;
    }

    for (i = 0; i < len; i++) {
        diff |= pa[i] ^ pb[i];
    }
    return diff == 0;
}

int nv_secure_str_equal(const char *a, const char *b)
{
    size_t la;
    size_t lb;
    size_t len;
    size_t i;
    unsigned char diff = 0;

    if (!a || !b) {
        return (a == b) ? 1 : 0;
    }

    la  = strlen(a);
    lb  = strlen(b);
    len = la > lb ? la : lb;
    diff = (unsigned char)(la ^ lb);

    for (i = 0; i < len; i++) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= ca ^ cb;
    }
    return diff == 0;
}

int nv_secure_is_weak_password(const char *password)
{
    static const char *const weak[] = {
        "",
        "123456",
        "nvadmin",
        "admin",
        "password",
        "12345678",
        "root",
        NULL
    };
    int i;

    if (!password) {
        return 1;
    }
    for (i = 0; weak[i]; i++) {
        if (nv_secure_str_equal(password, weak[i])) {
            return 1;
        }
    }
    if (strlen(password) < 8) {
        return 1;
    }
    return 0;
}
