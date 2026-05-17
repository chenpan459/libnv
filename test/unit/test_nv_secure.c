#include <nv_secure.h>
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
    check(nv_secure_str_equal("abc", "abc") == 1, "equal strings");
    check(nv_secure_str_equal("abc", "abd") == 0, "diff strings");
    check(nv_secure_str_equal("abc", "ab") == 0, "diff length");
    check(nv_secure_mem_equal("data", "data", 4) == 1, "mem equal");
    check(nv_secure_is_weak_password("123456") == 1, "weak 123456");
    check(nv_secure_is_weak_password("nvadmin") == 1, "weak nvadmin");
    check(nv_secure_is_weak_password("ChangeMe8!") == 0, "strong password");

    if (g_fail) {
        fprintf(stderr, "%d test(s) failed\n", g_fail);
        return 1;
    }
    printf("test_nv_secure: ok\n");
    return 0;
}
