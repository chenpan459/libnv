#include "nv_core_private.h"

#include <nv_dlopen.h>
#include <nv_log.h>

#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

static char *nv_loadlib_trim(char *s)
{
    char *end;

    if (!s) {
        return s;
    }
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;
    }
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static void nv_loadlib_strip_comment(char *line)
{
    int in_quote = 0;
    int escape = 0;

    for (; line && *line; line++) {
        if (escape) {
            escape = 0;
            continue;
        }
        if (*line == '\\' && in_quote) {
            escape = 1;
            continue;
        }
        if (*line == '"') {
            in_quote = !in_quote;
            continue;
        }
        if (*line == ';' && !in_quote) {
            *line = '\0';
            return;
        }
    }
}

static int nv_loadlib_parse_token(const char **src, char *buf, size_t size)
{
    const char *p;
    size_t      n = 0;

    if (!src || !*src || !buf || size == 0) {
        return NV_ERROR;
    }

    p = *src;
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0') {
        return NV_DECLINED;
    }

    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && p[1] != '\0') {
                p++;
            }
            if (n + 1 >= size) {
                return NV_ERROR;
            }
            buf[n++] = *p++;
        }
        if (*p != '"') {
            return NV_ERROR;
        }
        p++;
    } else {
        while (*p && !isspace((unsigned char)*p)) {
            if (n + 1 >= size) {
                return NV_ERROR;
            }
            buf[n++] = *p++;
        }
    }

    buf[n] = '\0';
    *src = p;
    return NV_OK;
}

static void nv_core_loadlibs_free_list(nv_core_loadlib_t *lib)
{
    nv_core_loadlib_t *next;

    while (lib) {
        next = lib->next;
        free(lib->path);
        free(lib->args);
        free(lib);
        lib = next;
    }
}

static int nv_loadlib_path_is_under_dir(const char *path, const char *dir)
{
    size_t len;

    if (!path || !dir || !dir[0]) {
        return 0;
    }
    len = strlen(dir);
    if (strncmp(path, dir, len) != 0) {
        return 0;
    }
    return (path[len] == '\0' || path[len] == '/');
}

static char *nv_core_loadlib_resolve_path(nv_core_ctx_t *ctx, const char *path)
{
    char  joined[PATH_MAX];
    char *out;

    if (!ctx || !path || !path[0]) {
        return NULL;
    }
    if (strstr(path, "..") != NULL) {
        return NULL;
    }

    if (path[0] == '/') {
        if (!ctx->opts.loadlib_allow_absolute) {
            nv_log_error("loadlib: absolute path denied: %s", path);
            return NULL;
        }
        if (ctx->opts.loadlib_dir && ctx->opts.loadlib_dir[0] &&
            !nv_loadlib_path_is_under_dir(path, ctx->opts.loadlib_dir)) {
            nv_log_error("loadlib: path outside plugin_dir: %s", path);
            return NULL;
        }
        return strdup(path);
    }

    if (strchr(path, '/') != NULL) {
        if (!ctx->opts.loadlib_allow_absolute) {
            nv_log_error("loadlib: relative path with '/' denied: %s", path);
            return NULL;
        }
        return strdup(path);
    }

    if (!ctx->opts.loadlib_dir || !ctx->opts.loadlib_dir[0]) {
        return strdup(path);
    }

    snprintf(joined, sizeof(joined), "%s/%s", ctx->opts.loadlib_dir, path);
    out = strdup(joined);
    return out;
}

static int nv_core_loadlibs_append(nv_core_ctx_t *ctx, const char *path,
                                   const char *args)
{
    nv_core_loadlib_t *lib;
    nv_core_loadlib_t **tail;
    char              *resolved;

    resolved = nv_core_loadlib_resolve_path(ctx, path);
    if (!resolved) {
        return NV_ERROR;
    }

    lib = (nv_core_loadlib_t *)calloc(1, sizeof(*lib));
    if (!lib) {
        free(resolved);
        return NV_ERROR;
    }

    lib->path = resolved;
    lib->args = strdup(args ? args : "");
    if (!lib->path || !lib->args) {
        free(lib->path);
        free(lib->args);
        free(lib);
        return NV_ERROR;
    }

    tail = &ctx->loadlibs;
    while (*tail) {
        tail = &(*tail)->next;
    }
    *tail = lib;
    return NV_OK;
}

static void nv_core_loadlib_block_core_signals(void)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

static void *nv_core_loadlib_thread_main(void *data)
{
    nv_core_loadlib_t *lib = (nv_core_loadlib_t *)data;
    int                rc;

    if (!lib || !lib->init) {
        return NULL;
    }

    nv_core_loadlib_block_core_signals();

    rc = lib->init(lib->ctx, lib->args ? lib->args : "");
    lib->init_rc = rc;
    lib->thread_exited = 1;

    if (rc != NV_OK) {
        nv_log_error("loadlib thread exited with error: %s", lib->path);
    } else {
        nv_log_info("loadlib thread exited: %s", lib->path);
    }
    return NULL;
}

int nv_core_loadlibs_parse_config(nv_core_ctx_t *ctx)
{
    FILE *fp;
    char  line[1024];
    int   lineno = 0;

    if (!ctx || !ctx->opts.config_file) {
        return NV_ERROR;
    }

    if (ctx->loadlibs_loaded) {
        nv_log_warning("loadlib reload is ignored after startup");
        return NV_OK;
    }

    nv_core_loadlibs_free_list(ctx->loadlibs);
    ctx->loadlibs = NULL;

    fp = fopen(ctx->opts.config_file, "r");
    if (!fp) {
        return NV_OK;
    }

    while (fgets(line, sizeof(line), fp)) {
        const char *p;
        char       *trimmed;
        char        path[512];
        char        args[512];
        int         rc;

        lineno++;
        nv_loadlib_strip_comment(line);
        trimmed = nv_loadlib_trim(line);
        if (*trimmed == '\0') {
            continue;
        }
        if (strncmp(trimmed, "loadlib", 7) != 0 ||
            (trimmed[7] != '\0' && !isspace((unsigned char)trimmed[7]))) {
            continue;
        }

        p = trimmed + 7;
        rc = nv_loadlib_parse_token(&p, path, sizeof(path));
        if (rc != NV_OK || path[0] == '\0') {
            nv_log_error("config loadlib parse failed at line %d", lineno);
            fclose(fp);
            return NV_ERROR;
        }

        rc = nv_loadlib_parse_token(&p, args, sizeof(args));
        if (rc == NV_DECLINED) {
            args[0] = '\0';
        } else if (rc != NV_OK) {
            nv_log_error("config loadlib args parse failed at line %d", lineno);
            fclose(fp);
            return NV_ERROR;
        }

        p = nv_loadlib_trim((char *)p);
        if (*p != '\0') {
            nv_log_error("config loadlib has unexpected token at line %d", lineno);
            fclose(fp);
            return NV_ERROR;
        }

        if (nv_core_loadlibs_append(ctx, path, args) != NV_OK) {
            fclose(fp);
            return NV_ERROR;
        }
    }

    fclose(fp);
    return NV_OK;
}

static int nv_core_loadlib_join(nv_core_loadlib_t *lib, int timeout_ms)
{
    int rc;
    int waited_ms = 0;

    if (!lib || !lib->thread_started) {
        return NV_OK;
    }

    if (timeout_ms <= 0) {
        timeout_ms = 3000;
    }

    for (;;) {
        rc = pthread_tryjoin_np(lib->thread, NULL);
        if (rc == 0) {
            lib->thread_started = 0;
            return NV_OK;
        }
        if (rc != EBUSY) {
            nv_log_warning("loadlib thread join failed: %s (%s)",
                           lib->path, strerror(rc));
            lib->thread_started = 0;
            return NV_ERROR;
        }
        if (waited_ms >= timeout_ms) {
            break;
        }
        usleep(100000);
        waited_ms += 100;
    }

    nv_log_warning("loadlib thread still running, skip dlclose: %s", lib->path);
    pthread_detach(lib->thread);
    lib->thread_started = 0;
    return NV_BUSY;
}

static void nv_core_loadlibs_close_loaded(nv_core_loadlib_t *lib,
                                          int timeout_ms)
{
    int joined = NV_OK;

    if (!lib) {
        return;
    }
    nv_core_loadlibs_close_loaded(lib->next, timeout_ms);
    if (lib->handle) {
        if (lib->cleanup) {
            lib->cleanup(g_core_ctx);
        }
        joined = nv_core_loadlib_join(lib, timeout_ms);
        if (joined == NV_OK || joined == NV_ERROR) {
            nv_dlclose(lib->handle);
            lib->handle = NULL;
        }
    }
}

int nv_core_loadlibs_load(nv_core_ctx_t *ctx)
{
    nv_core_loadlib_t *lib;

    if (!ctx) {
        return NV_ERROR;
    }
    if (ctx->loadlibs_loaded) {
        return NV_OK;
    }

    /*
     * loadlib 线程早于 core signalfd 注册启动，必须先屏蔽 core 管理的信号，
     * 避免 SIGTERM 等被业务线程按默认动作处理并直接终止进程。
     */
    nv_core_loadlib_block_core_signals();

    for (lib = ctx->loadlibs; lib; lib = lib->next) {
        lib->handle = nv_dlopen(lib->path);
        if (!lib->handle) {
            nv_log_error("loadlib dlopen failed: %s (%s)",
                         lib->path, nv_dlerror());
            nv_core_loadlibs_close_loaded(ctx->loadlibs,
                                          ctx->opts.shutdown_timeout_sec * 1000);
            return NV_ERROR;
        }

        dlerror();
        lib->init = (nv_core_loadlib_init_pt)
            nv_dlsym(lib->handle, NV_CORE_LOADLIB_INIT_SYMBOL);
        if (!lib->init) {
            nv_log_error("loadlib missing symbol %s: %s",
                         NV_CORE_LOADLIB_INIT_SYMBOL, lib->path);
            nv_core_loadlibs_close_loaded(ctx->loadlibs,
                                          ctx->opts.shutdown_timeout_sec * 1000);
            return NV_ERROR;
        }

        dlerror();
        lib->cleanup = (nv_core_loadlib_cleanup_pt)
            nv_dlsym(lib->handle, NV_CORE_LOADLIB_CLEANUP_SYMBOL);

        lib->ctx = ctx;
        lib->init_rc = NV_OK;
        lib->thread_exited = 0;
        if (pthread_create(&lib->thread, NULL,
                           nv_core_loadlib_thread_main, lib) != 0) {
            nv_log_error("loadlib thread create failed: %s", lib->path);
            nv_core_loadlibs_close_loaded(ctx->loadlibs,
                                          ctx->opts.shutdown_timeout_sec * 1000);
            return NV_ERROR;
        }
        lib->thread_started = 1;

        nv_log_info("loadlib thread started: %s", lib->path);
    }

    ctx->loadlibs_loaded = 1;
    return NV_OK;
}

void nv_core_loadlibs_unload(nv_core_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    nv_core_loadlibs_close_loaded(ctx->loadlibs,
                                  ctx->opts.shutdown_timeout_sec * 1000);
    nv_core_loadlibs_free_list(ctx->loadlibs);
    ctx->loadlibs = NULL;
    ctx->loadlibs_loaded = 0;
}
