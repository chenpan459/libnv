#ifndef _NV_WATCHDOG_H_INCLUDED_
#define _NV_WATCHDOG_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <nv_config.h>
#include <nv_base.h>

/** 打开硬件看门狗设备（如 /dev/watchdog），成功返回 NV_OK */
int nv_watchdog_open(const char *device);

/** 喂狗（写 magic close 字符或 ioctl），未打开时返回 NV_DECLINED */
int nv_watchdog_feed(void);

void nv_watchdog_close(void);

/** 通过 shell 命令喂狗（配置项 watchdog_cmd），cmd 为空则 NV_DECLINED */
int nv_watchdog_feed_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
