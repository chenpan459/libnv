#include <nv_log.h>
#include <stdio.h>
#include <time.h>

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
    nv_log_queue_stats_t st;
    int                  i;

    if (nv_log_init_file("/tmp/nv_log_unit_test.log") != 0) {
        fprintf(stderr, "nv_log_init_file failed\n");
        return 1;
    }

    nv_log_set_level(NV_LOG_LEVEL_DEBUG);
    nv_log_set_overflow_policy(NV_LOG_OVERFLOW_DROP);

    for (i = 0; i < 32; i++) {
        nv_log_info("unit test line %d", i);
    }

    {
        struct timespec ts = {0, 200000000L};
        nanosleep(&ts, NULL);
    }

    nv_log_get_queue_stats(&st);
    check(st.capacity > 0, "queue capacity");
    check(st.overflow != NULL, "overflow policy string");

    nv_log_close();

    if (g_fail) {
        fprintf(stderr, "%d test(s) failed\n", g_fail);
        return 1;
    }
    printf("test_nv_log: ok (capacity=%zu dropped=%llu)\n",
           st.capacity, (unsigned long long)st.dropped);
    return 0;
}
