#ifndef _NV_CORE_CLI_H_INCLUDED_
#define _NV_CORE_CLI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

#define NV_CORE_CLI_LINE_MAX     512
#define NV_CORE_CLI_OUT_MAX      4096
#define NV_CLI_CLOSE_SESSION     2   /* 仅断开当前 CLI 会话（Telnet） */

#define NV_CORE_CLI_NAME_MAX     32
#define NV_CORE_CLI_HELP_MAX     128

typedef int (*nv_core_cli_handler_t)(nv_core_ctx_t *ctx, int fd,
                                     int argc, char **argv);

typedef struct nv_core_cli_cmd_def_s {
    const char             *name;
    const char             *help;      /* help 列表中的简短说明 */
    nv_core_cli_handler_t   handler;
    void                   *userdata;
} nv_core_cli_cmd_def_t;

int  nv_core_cli_write(int fd, const char *fmt, ...);
void nv_core_cli_print_banner(int fd);
void nv_core_cli_print_prompt(int fd);

void nv_core_cli_init(void);
void nv_core_cli_cleanup(void);

/* 注册/注销命令；name 唯一，重复注册返回 NV_ERROR */
int nv_core_cli_register(const nv_core_cli_cmd_def_t *cmd);
int nv_core_cli_unregister(const char *name);

int nv_core_cli_execute_line(nv_core_ctx_t *ctx, int fd, const char *line,
                             int interactive);
int nv_core_cli_tab_complete(int fd, char *line, size_t line_max, size_t *line_len);

/* 便捷宏：模块加载时自动注册（需链接进可执行文件/库） */
#define NV_CORE_CLI_REGISTER(name, help, func)                          \
    static nv_core_cli_cmd_def_t _nv_cli_cmd_##name = {                  \
        #name, help, func, NULL                                          \
    };                                                                   \
    __attribute__((constructor))                                           \
    static void _nv_cli_register_##name(void) {                           \
        nv_core_cli_register(&_nv_cli_cmd_##name);                       \
    }

#ifdef __cplusplus
}
#endif

#endif
