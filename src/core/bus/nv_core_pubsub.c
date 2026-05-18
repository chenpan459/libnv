#include "nv_core_private.h"

#include <nv_log.h>

#include <stdlib.h>
#include <string.h>

struct nv_core_pubsub_s {
    unsigned long               id;
    char                       *topic;
    nv_core_pubsub_handler_pt   handler;
    void                       *user_data;
    int                         active;
    int                         calls;
    struct nv_core_pubsub_s    *next;
};

typedef struct {
    nv_core_pubsub_t           *sub;
    nv_core_pubsub_handler_pt   handler;
    void                       *user_data;
    char                       *topic;
} nv_core_pubsub_call_t;

static int nv_core_pubsub_init(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return NV_ERROR;
    }
    if (ctx->pubsub_inited) {
        return NV_OK;
    }

    if (pthread_mutex_init(&ctx->pubsub_mutex, NULL) != 0) {
        return NV_ERROR;
    }
    if (pthread_cond_init(&ctx->pubsub_cond, NULL) != 0) {
        pthread_mutex_destroy(&ctx->pubsub_mutex);
        return NV_ERROR;
    }

    ctx->pubsub_inited = 1;
    ctx->pubsub_next_id = 1;
    return NV_OK;
}

int nv_core_pubsub_subscribe(nv_core_ctx_t *ctx, const char *topic,
                             nv_core_pubsub_handler_pt handler,
                             void *user_data, void **sub_handle)
{
    nv_core_pubsub_t *sub;

    if (!ctx || !topic || !topic[0] || !handler) {
        return NV_ERROR;
    }
    if (nv_core_pubsub_init(ctx) != NV_OK) {
        return NV_ERROR;
    }

    sub = (nv_core_pubsub_t *)calloc(1, sizeof(*sub));
    if (!sub) {
        return NV_ERROR;
    }
    sub->topic = strdup(topic);
    if (!sub->topic) {
        free(sub);
        return NV_ERROR;
    }
    sub->handler = handler;
    sub->user_data = user_data;
    sub->active = 1;

    pthread_mutex_lock(&ctx->pubsub_mutex);
    sub->id = ctx->pubsub_next_id++;
    sub->next = ctx->pubsubs;
    ctx->pubsubs = sub;
    pthread_mutex_unlock(&ctx->pubsub_mutex);

    if (sub_handle) {
        *sub_handle = sub;
    }
    nv_log_info("pubsub subscribe topic=%s id=%lu", topic, sub->id);
    return NV_OK;
}

int nv_core_pubsub_unsubscribe(nv_core_ctx_t *ctx, void *sub_handle)
{
    nv_core_pubsub_t *sub = (nv_core_pubsub_t *)sub_handle;

    if (!ctx || !sub || !ctx->pubsub_inited) {
        return NV_ERROR;
    }

    pthread_mutex_lock(&ctx->pubsub_mutex);
    sub->active = 0;
    while (sub->calls > 0) {
        pthread_cond_wait(&ctx->pubsub_cond, &ctx->pubsub_mutex);
    }
    pthread_mutex_unlock(&ctx->pubsub_mutex);

    nv_log_info("pubsub unsubscribe topic=%s id=%lu", sub->topic, sub->id);
    return NV_OK;
}

int nv_core_pubsub_publish(nv_core_ctx_t *ctx, const char *topic,
                           const void *data, size_t len)
{
    nv_core_pubsub_t      *sub;
    nv_core_pubsub_call_t *calls = NULL;
    size_t                 count = 0;
    size_t                 i = 0;

    if (!ctx || !topic || !topic[0]) {
        return NV_ERROR;
    }
    if (nv_core_pubsub_init(ctx) != NV_OK) {
        return NV_ERROR;
    }

    pthread_mutex_lock(&ctx->pubsub_mutex);
    for (sub = ctx->pubsubs; sub; sub = sub->next) {
        if (sub->active && strcmp(sub->topic, topic) == 0) {
            count++;
        }
    }

    if (count > 0) {
        calls = (nv_core_pubsub_call_t *)calloc(count, sizeof(*calls));
        if (!calls) {
            pthread_mutex_unlock(&ctx->pubsub_mutex);
            return NV_ERROR;
        }

        for (sub = ctx->pubsubs; sub; sub = sub->next) {
            if (sub->active && strcmp(sub->topic, topic) == 0) {
                sub->calls++;
                calls[i].sub = sub;
                calls[i].handler = sub->handler;
                calls[i].user_data = sub->user_data;
                calls[i].topic = sub->topic;
                i++;
            }
        }
    }
    pthread_mutex_unlock(&ctx->pubsub_mutex);

    for (i = 0; i < count; i++) {
        calls[i].handler(ctx, calls[i].topic, data, len, calls[i].user_data);

        pthread_mutex_lock(&ctx->pubsub_mutex);
        calls[i].sub->calls--;
        pthread_cond_broadcast(&ctx->pubsub_cond);
        pthread_mutex_unlock(&ctx->pubsub_mutex);
    }

    free(calls);
    return NV_OK;
}

void nv_core_pubsub_cleanup(nv_core_ctx_t *ctx)
{
    nv_core_pubsub_t *sub;
    nv_core_pubsub_t *next;

    if (!ctx || !ctx->pubsub_inited) {
        return;
    }

    pthread_mutex_lock(&ctx->pubsub_mutex);
    for (sub = ctx->pubsubs; sub; sub = sub->next) {
        sub->active = 0;
        while (sub->calls > 0) {
            pthread_cond_wait(&ctx->pubsub_cond, &ctx->pubsub_mutex);
        }
    }
    pthread_mutex_unlock(&ctx->pubsub_mutex);

    sub = ctx->pubsubs;
    while (sub) {
        next = sub->next;
        free(sub->topic);
        free(sub);
        sub = next;
    }
    ctx->pubsubs = NULL;

    pthread_cond_destroy(&ctx->pubsub_cond);
    pthread_mutex_destroy(&ctx->pubsub_mutex);
    ctx->pubsub_inited = 0;
    ctx->pubsub_next_id = 0;
}
