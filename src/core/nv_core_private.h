#ifndef _NV_CORE_PRIVATE_H_INCLUDED_
#define _NV_CORE_PRIVATE_H_INCLUDED_

#include "nv_core.h"
#include "nv_event.h"

#ifdef __cplusplus
extern "C" {
#endif

extern nv_core_ctx_t        *g_core_ctx;
extern const nv_core_hooks_t *g_core_hooks;
extern nv_event_ext_t        g_idle_ev;
extern char                **g_saved_argv;
extern int                   g_saved_argc;

void nv_core_set_defaults(nv_core_ctx_t *ctx);
void nv_core_cfg_set_str(char **slot, const char *val);
int  nv_core_parse_log_level(const char *v);

int  nv_core_log_init(nv_core_ctx_t *ctx);
int  nv_core_pidfile_create(nv_core_ctx_t *ctx);
void nv_core_pidfile_remove(nv_core_ctx_t *ctx);

void nv_core_on_signal(int signum);
int  nv_core_signals_register_loop(nv_core_ctx_t *ctx);
void nv_core_apply_rlimits(nv_core_ctx_t *ctx);

void nv_core_check_shutdown_timeout(nv_core_ctx_t *ctx);
void nv_core_idle_handler(nv_loop_t *loop, void *ev, void *data);

void nv_core_free_saved_argv(void);
void nv_core_save_argv(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
