#include "nv_core_private.h"

#include <nv_log.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define NV_PUBSUB_TOPIC_MAX  64
#define NV_PUBSUB_DATA_MAX   512
#define NV_PUBSUB_MAGIC      0x4e565053U /* NVPS */

typedef enum {
    NV_PUBSUB_OP_PUB = 1,
    NV_PUBSUB_OP_SUB = 2,
    NV_PUBSUB_OP_UNSUB = 3
} nv_pubsub_op_t;

typedef struct {
    unsigned int magic;
    unsigned int op;
    unsigned int len;
    char         topic[NV_PUBSUB_TOPIC_MAX];
    char         data[NV_PUBSUB_DATA_MAX];
} nv_pubsub_packet_t;

struct nv_core_pubsub_s {
    unsigned long               id;
    char                       *topic;
    nv_core_pubsub_handler_pt   handler;
    void                       *user_data;
    int                         active;
    int                         calls;
    struct nv_core_pubsub_s    *next;
};

struct nv_core_pubsub_ipc_client_s {
    char                            topic[NV_PUBSUB_TOPIC_MAX];
    struct sockaddr_un              addr;
    socklen_t                       addrlen;
    struct nv_core_pubsub_ipc_client_s *next;
};

typedef struct {
    nv_core_pubsub_t           *sub;
    nv_core_pubsub_handler_pt   handler;
    void                       *user_data;
    char                       *topic;
} nv_core_pubsub_call_t;

static int nv_pubsub_addr_equal(const struct sockaddr_un *a, socklen_t alen,
                                const struct sockaddr_un *b, socklen_t blen)
{
    if (!a || !b || alen != blen) {
        return 0;
    }
    return memcmp(a, b, alen) == 0;
}

static void nv_pubsub_fill_packet(nv_pubsub_packet_t *pkt, nv_pubsub_op_t op,
                                  const char *topic, const void *data,
                                  size_t len)
{
    memset(pkt, 0, sizeof(*pkt));
    pkt->magic = NV_PUBSUB_MAGIC;
    pkt->op = (unsigned int)op;
    snprintf(pkt->topic, sizeof(pkt->topic), "%s", topic ? topic : "");
    if (data && len > 0) {
        if (len > NV_PUBSUB_DATA_MAX) {
            len = NV_PUBSUB_DATA_MAX;
        }
        memcpy(pkt->data, data, len);
        pkt->len = (unsigned int)len;
    }
}

static int nv_pubsub_send_packet(int fd, const struct sockaddr_un *addr,
                                 socklen_t addrlen, const nv_pubsub_packet_t *pkt)
{
    ssize_t n;

    n = sendto(fd, pkt, sizeof(*pkt), 0, (const struct sockaddr *)addr, addrlen);
    return n == (ssize_t)sizeof(*pkt) ? NV_OK : NV_ERROR;
}

static void nv_core_pubsub_remove_ipc_client(nv_core_ctx_t *ctx,
                                             const char *topic,
                                             const struct sockaddr_un *addr,
                                             socklen_t addrlen)
{
    nv_core_pubsub_ipc_client_t **cur;
    nv_core_pubsub_ipc_client_t  *client;

    cur = &ctx->pubsub_ipc_clients;
    while (*cur) {
        client = *cur;
        if (strcmp(client->topic, topic) == 0 &&
            nv_pubsub_addr_equal(&client->addr, client->addrlen, addr, addrlen)) {
            *cur = client->next;
            free(client);
            return;
        }
        cur = &client->next;
    }
}

static int nv_core_pubsub_add_ipc_client(nv_core_ctx_t *ctx, const char *topic,
                                         const struct sockaddr_un *addr,
                                         socklen_t addrlen)
{
    nv_core_pubsub_ipc_client_t *client;

    nv_core_pubsub_remove_ipc_client(ctx, topic, addr, addrlen);

    client = (nv_core_pubsub_ipc_client_t *)calloc(1, sizeof(*client));
    if (!client) {
        return NV_ERROR;
    }
    snprintf(client->topic, sizeof(client->topic), "%s", topic);
    memcpy(&client->addr, addr, addrlen);
    client->addrlen = addrlen;
    client->next = ctx->pubsub_ipc_clients;
    ctx->pubsub_ipc_clients = client;
    return NV_OK;
}

static void nv_core_pubsub_publish_ipc(nv_core_ctx_t *ctx, const char *topic,
                                       const void *data, size_t len)
{
    nv_core_pubsub_ipc_client_t *client;
    nv_pubsub_packet_t           pkt;

    if (!ctx || ctx->pubsub_ipc_fd < 0) {
        return;
    }

    nv_pubsub_fill_packet(&pkt, NV_PUBSUB_OP_PUB, topic, data, len);

    pthread_mutex_lock(&ctx->pubsub_mutex);
    for (client = ctx->pubsub_ipc_clients; client; client = client->next) {
        if (strcmp(client->topic, topic) == 0) {
            nv_pubsub_send_packet(ctx->pubsub_ipc_fd, &client->addr,
                                  client->addrlen, &pkt);
        }
    }
    pthread_mutex_unlock(&ctx->pubsub_mutex);
}

static void *nv_core_pubsub_ipc_thread_main(void *data)
{
    nv_core_ctx_t      *ctx = (nv_core_ctx_t *)data;
    nv_pubsub_packet_t  pkt;
    struct sockaddr_un  peer;
    socklen_t           peerlen;
    ssize_t             n;

    while (ctx && ctx->pubsub_ipc_running) {
        peerlen = sizeof(peer);
        n = recvfrom(ctx->pubsub_ipc_fd, &pkt, sizeof(pkt), 0,
                     (struct sockaddr *)&peer, &peerlen);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000);
                continue;
            }
            break;
        }
        if (n != (ssize_t)sizeof(pkt) || pkt.magic != NV_PUBSUB_MAGIC ||
            pkt.topic[0] == '\0') {
            continue;
        }
        if (pkt.len > NV_PUBSUB_DATA_MAX) {
            pkt.len = NV_PUBSUB_DATA_MAX;
        }

        if (pkt.op == NV_PUBSUB_OP_SUB) {
            pthread_mutex_lock(&ctx->pubsub_mutex);
            nv_core_pubsub_add_ipc_client(ctx, pkt.topic, &peer, peerlen);
            pthread_mutex_unlock(&ctx->pubsub_mutex);
            nv_log_info("pubsub ipc subscribe topic=%s", pkt.topic);
        } else if (pkt.op == NV_PUBSUB_OP_UNSUB) {
            pthread_mutex_lock(&ctx->pubsub_mutex);
            nv_core_pubsub_remove_ipc_client(ctx, pkt.topic, &peer, peerlen);
            pthread_mutex_unlock(&ctx->pubsub_mutex);
            nv_log_info("pubsub ipc unsubscribe topic=%s", pkt.topic);
        } else if (pkt.op == NV_PUBSUB_OP_PUB) {
            nv_core_pubsub_publish(ctx, pkt.topic, pkt.data, pkt.len);
        }
    }

    return NULL;
}

static int nv_core_pubsub_start_ipc(nv_core_ctx_t *ctx)
{
    struct sockaddr_un addr;
    int                fd;
    const char        *path;

    if (!ctx || ctx->pubsub_ipc_thread_started) {
        return NV_OK;
    }

    path = ctx->opts.pubsub_socket ? ctx->opts.pubsub_socket : NV_CORE_DEFAULT_PUBSUB_SOCKET;
    fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        nv_log_warning("pubsub ipc socket create failed: %s", strerror(errno));
        return NV_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    unlink(path);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        nv_log_warning("pubsub ipc bind failed: %s (%s)", path, strerror(errno));
        close(fd);
        return NV_ERROR;
    }

    ctx->pubsub_ipc_fd = fd;
    ctx->pubsub_ipc_running = 1;
    if (pthread_create(&ctx->pubsub_ipc_thread, NULL,
                       nv_core_pubsub_ipc_thread_main, ctx) != 0) {
        close(fd);
        ctx->pubsub_ipc_fd = -1;
        ctx->pubsub_ipc_running = 0;
        return NV_ERROR;
    }

    ctx->pubsub_ipc_thread_started = 1;
    nv_log_info("pubsub ipc broker listening: %s", path);
    return NV_OK;
}

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
    ctx->pubsub_ipc_fd = -1;
    if (nv_core_pubsub_start_ipc(ctx) != NV_OK) {
        nv_log_warning("pubsub ipc broker disabled");
    }
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
    if (len > NV_PUBSUB_DATA_MAX) {
        len = NV_PUBSUB_DATA_MAX;
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
    nv_core_pubsub_publish_ipc(ctx, topic, data, len);
    return NV_OK;
}

static void nv_core_pubsub_free_ipc_clients(nv_core_ctx_t *ctx)
{
    nv_core_pubsub_ipc_client_t *client;
    nv_core_pubsub_ipc_client_t *next;

    client = ctx->pubsub_ipc_clients;
    while (client) {
        next = client->next;
        free(client);
        client = next;
    }
    ctx->pubsub_ipc_clients = NULL;
}

void nv_core_pubsub_cleanup(nv_core_ctx_t *ctx)
{
    nv_core_pubsub_t *sub;
    nv_core_pubsub_t *next;

    if (!ctx || !ctx->pubsub_inited) {
        return;
    }

    ctx->pubsub_ipc_running = 0;
    if (ctx->pubsub_ipc_fd >= 0) {
        close(ctx->pubsub_ipc_fd);
        ctx->pubsub_ipc_fd = -1;
    }
    if (ctx->pubsub_ipc_thread_started) {
        pthread_join(ctx->pubsub_ipc_thread, NULL);
        ctx->pubsub_ipc_thread_started = 0;
    }
    if (ctx->opts.pubsub_socket) {
        unlink(ctx->opts.pubsub_socket);
    }

    pthread_mutex_lock(&ctx->pubsub_mutex);
    for (sub = ctx->pubsubs; sub; sub = sub->next) {
        sub->active = 0;
        while (sub->calls > 0) {
            pthread_cond_wait(&ctx->pubsub_cond, &ctx->pubsub_mutex);
        }
    }
    nv_core_pubsub_free_ipc_clients(ctx);
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

static int nv_pubsub_make_client_socket(struct sockaddr_un *addr,
                                        char *path, size_t path_size)
{
    int fd;

    fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        return -1;
    }

    snprintf(path, path_size, "/tmp/nv_pubsub_cli_%ld_%ld.sock",
             (long)getpid(), (long)pthread_self());
    unlink(path);
    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    snprintf(addr->sun_path, sizeof(addr->sun_path), "%s", path);
    if (bind(fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void nv_pubsub_fill_server_addr(struct sockaddr_un *addr,
                                       const char *server_socket)
{
    memset(addr, 0, sizeof(*addr));
    addr->sun_family = AF_UNIX;
    snprintf(addr->sun_path, sizeof(addr->sun_path), "%s",
             server_socket ? server_socket : NV_CORE_DEFAULT_PUBSUB_SOCKET);
}

int nv_core_pubsub_client_publish(const char *server_socket,
                                  const char *topic,
                                  const void *data, size_t len)
{
    int                 fd;
    struct sockaddr_un  local;
    struct sockaddr_un  server;
    char                path[108];
    nv_pubsub_packet_t  pkt;
    int                 rc;

    if (!topic || !topic[0]) {
        return NV_ERROR;
    }

    fd = nv_pubsub_make_client_socket(&local, path, sizeof(path));
    if (fd < 0) {
        return NV_ERROR;
    }

    nv_pubsub_fill_server_addr(&server, server_socket);
    nv_pubsub_fill_packet(&pkt, NV_PUBSUB_OP_PUB, topic, data, len);
    rc = nv_pubsub_send_packet(fd, &server, sizeof(server), &pkt);
    close(fd);
    unlink(path);
    return rc;
}

int nv_core_pubsub_client_subscribe(const char *server_socket,
                                    const char *topic,
                                    nv_core_pubsub_client_handler_pt handler,
                                    void *user_data,
                                    volatile int *stop)
{
    int                 fd;
    struct sockaddr_un  local;
    struct sockaddr_un  server;
    char                path[108];
    nv_pubsub_packet_t  pkt;
    struct pollfd       pfd;

    if (!topic || !topic[0] || !handler) {
        return NV_ERROR;
    }

    fd = nv_pubsub_make_client_socket(&local, path, sizeof(path));
    if (fd < 0) {
        return NV_ERROR;
    }

    nv_pubsub_fill_server_addr(&server, server_socket);
    nv_pubsub_fill_packet(&pkt, NV_PUBSUB_OP_SUB, topic, NULL, 0);
    if (nv_pubsub_send_packet(fd, &server, sizeof(server), &pkt) != NV_OK) {
        close(fd);
        unlink(path);
        return NV_ERROR;
    }

    while (!stop || !*stop) {
        ssize_t n;

        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        if (poll(&pfd, 1, 500) <= 0) {
            continue;
        }
        n = recv(fd, &pkt, sizeof(pkt), 0);
        if (n == (ssize_t)sizeof(pkt) && pkt.magic == NV_PUBSUB_MAGIC &&
            pkt.op == NV_PUBSUB_OP_PUB && strcmp(pkt.topic, topic) == 0) {
            if (pkt.len > NV_PUBSUB_DATA_MAX) {
                pkt.len = NV_PUBSUB_DATA_MAX;
            }
            handler(pkt.topic, pkt.data, pkt.len, user_data);
        }
    }

    nv_pubsub_fill_packet(&pkt, NV_PUBSUB_OP_UNSUB, topic, NULL, 0);
    nv_pubsub_send_packet(fd, &server, sizeof(server), &pkt);
    close(fd);
    unlink(path);
    return NV_OK;
}
