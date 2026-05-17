#ifndef _NV_CORE_CLI_H_INCLUDED_
#define _NV_CORE_CLI_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "nv_core.h"

#define NV_CORE_CLI_LINE_MAX     512
#define NV_CORE_CLI_OUT_MAX      4096
#define NV_CLI_CLOSE_SESSION     2   /* 仅断开当前 CLI 会话（Telnet） */

int  nv_core_cli_write(int fd, const char *fmt, ...);
void nv_core_cli_print_banner(int fd);
void nv_core_cli_print_prompt(int fd);

/* 执行一行命令，结果写入 fd；interactive=1 时 quit/exit 仅断开会话 */
int nv_core_cli_execute_line(nv_core_ctx_t *ctx, int fd, const char *line,
                             int interactive);

/* Tab 补全第一词（命令名），更新 line/line_len，必要时向 fd 输出候选列表 */
int nv_core_cli_tab_complete(int fd, char *line, size_t line_max, size_t *line_len);

#ifdef __cplusplus
}
#endif

#endif
