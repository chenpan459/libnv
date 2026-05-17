/************************************************
 * @文件名: nvd.c
 * @功能: libnv 主进程守护程序示例入口
 ***********************************************/

#include <nv_main.h>
#include <stdio.h>

static int app_business_init(nv_main_ctx_t *ctx)
{
    if (nv_main_business_init(ctx) != NV_OK) {
        return NV_ERROR;
    }
    nv_log_info("application business ready");
    return NV_OK;
}

static void app_business_cleanup(nv_main_ctx_t *ctx)
{
    nv_log_info("application business cleanup");
    nv_main_business_cleanup(ctx);
}

static void app_on_idle(nv_main_ctx_t *ctx)
{
    (void)ctx;
    /* 空闲检测：可在此做心跳、统计、资源回收 */
}

int main(int argc, char **argv)
{
    nv_main_ctx_t   ctx;
    nv_main_hooks_t hooks = {
        .on_business_init    = app_business_init,
        .on_business_cleanup = app_business_cleanup,
        .on_reload           = NULL,
        .on_idle             = app_on_idle,
    };

    memset(&ctx, 0, sizeof(ctx));
    return (nv_main_run(&ctx, argc, argv, &hooks) == NV_OK) ? 0 : 1;
}
