#include <nv_core.h>
#include <nv_log.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char g_alpha_args[256];
static volatile int g_alpha_running;
static void *g_alpha_sub;

static void loadlib_alpha_on_beta(nv_core_ctx_t *ctx, const char *topic,
                                  const void *data, size_t len,
                                  void *user_data)
{
    (void)ctx;
    (void)user_data;

    nv_log_info("loadlib_alpha received topic=%s msg=\"%.*s\"",
                topic, (int)len, data ? (const char *)data : "");
}

int nv_loadlib_init(nv_core_ctx_t *ctx, const char *args)
{
    int seq = 0;
    char msg[128];

    snprintf(g_alpha_args, sizeof(g_alpha_args), "%s", args ? args : "");
    g_alpha_running = 1;
    nv_log_info("loadlib_alpha business started args=\"%s\"", g_alpha_args);

    if (nv_core_pubsub_subscribe(ctx, "beta/status",
                                 loadlib_alpha_on_beta, NULL,
                                 &g_alpha_sub) != NV_OK) {
        nv_log_warning("loadlib_alpha subscribe beta/status failed");
    }

    while (g_alpha_running && !nv_core_is_quitting(ctx)) {
        snprintf(msg, sizeof(msg), "alpha heartbeat %d", ++seq);
        nv_log_info("loadlib_alpha publish topic=alpha/status msg=\"%s\"", msg);
        nv_core_pubsub_publish(ctx, "alpha/status", msg, strlen(msg));
        sleep(1);
    }

    nv_log_info("loadlib_alpha business stopped");
    return NV_OK;
}

void nv_loadlib_cleanup(nv_core_ctx_t *ctx)
{
    (void)ctx;

    g_alpha_running = 0;
    if (g_alpha_sub) {
        nv_core_pubsub_unsubscribe(ctx, g_alpha_sub);
        g_alpha_sub = NULL;
    }
    nv_log_info("loadlib_alpha cleanup args=\"%s\"", g_alpha_args);
}
