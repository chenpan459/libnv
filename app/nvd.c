/************************************************
 * @文件名: nvd.c
 * @功能: libnv 主进程守护程序示例入口
 ***********************************************/

#include <nv_core.h>
#include <nv_core_health.h>
#include <stdio.h>

static int app_business_init(nv_core_ctx_t *ctx)
{
    if (nv_core_business_init(ctx) != NV_OK) {
        return NV_ERROR;
    }
    nv_log_info("application business ready");
    return NV_OK;
}

static void app_business_cleanup(nv_core_ctx_t *ctx)
{
    nv_log_info("application business cleanup");
    nv_core_business_cleanup(ctx);
}

static void app_on_idle(nv_core_ctx_t *ctx)
{
    static unsigned long tick;

    if (!ctx) {
        return;
    }
    /* 约每 1000 次 idle 打印一次运行时统计（调试用） */
    if (++tick % 1000 == 0) {
        nv_core_runtime_stats_t st;
        nv_core_get_runtime_stats(ctx, &st);
        nv_log_debug("runtime uptime=%lus loop_events=%lu",
                     st.uptime_sec, st.loop_events);
    }
}

int main(int argc, char **argv)
{
    nv_core_ctx_t   ctx;
    nv_core_hooks_t hooks = {
        .on_business_init    = app_business_init,
        .on_business_cleanup = app_business_cleanup,
        .on_reload           = NULL,
        .on_idle             = app_on_idle,
    };

    memset(&ctx, 0, sizeof(ctx));
    return (nv_core_run(&ctx, argc, argv, &hooks) == NV_OK) ? 0 : 1;
}
