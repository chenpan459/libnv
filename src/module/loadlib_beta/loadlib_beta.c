#include <nv_core.h>
#include <nv_log.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_beta_init_count;
static volatile int g_beta_running;
static void *g_beta_sub;

static void loadlib_beta_on_alpha(nv_core_ctx_t *ctx, const char *topic,
                                  const void *data, size_t len,
                                  void *user_data)
{
    (void)ctx;
    (void)user_data;

    nv_log_info("loadlib_beta received topic=%s msg=\"%.*s\"",
                topic, (int)len, data ? (const char *)data : "");
}

int nv_loadlib_init(nv_core_ctx_t *ctx, const char *args)
{
    int seq = 0;
    char msg[128];

    g_beta_init_count++;
    g_beta_running = 1;
    nv_log_info("loadlib_beta business started count=%d args=\"%s\"",
                g_beta_init_count, args ? args : "");

    if (nv_core_pubsub_subscribe(ctx, "alpha/status",
                                 loadlib_beta_on_alpha, NULL,
                                 &g_beta_sub) != NV_OK) {
        nv_log_warning("loadlib_beta subscribe alpha/status failed");
    }

    while (g_beta_running && !nv_core_is_quitting(ctx)) {
        snprintf(msg, sizeof(msg), "beta heartbeat %d", ++seq);
        nv_core_pubsub_publish(ctx, "beta/status", msg, strlen(msg));
        sleep(1);
    }

    nv_log_info("loadlib_beta business stopped count=%d", g_beta_init_count);
    return NV_OK;
}

void nv_loadlib_cleanup(nv_core_ctx_t *ctx)
{
    (void)ctx;

    g_beta_running = 0;
    if (g_beta_sub) {
        nv_core_pubsub_unsubscribe(ctx, g_beta_sub);
        g_beta_sub = NULL;
    }
    nv_log_info("loadlib_beta cleanup count=%d", g_beta_init_count);
}
