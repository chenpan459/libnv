/************************************************
 * @文件名: nv_base.c
 * @功能: libnv 核心层实现（模块注册与周期管理）
 ***********************************************/

#include "nv_base.h"
#include <string.h>
#include <stdlib.h>

static nv_module_t *g_modules[NV_MODULE_MAX];
static int g_module_count = 0;
static nv_cycle_t *g_cycle = NULL;

int nv_base_register_module(nv_module_t *module)
{
    if (!module || g_module_count >= NV_MODULE_MAX) {
        return NV_ERROR;
    }
    g_modules[g_module_count++] = module;
    return NV_OK;
}

int nv_base_init(nv_conf_t *conf)
{
    int i;
    for (i = 0; i < g_module_count; i++) {
        if (g_modules[i] && g_modules[i]->init) {
            if (g_modules[i]->init(conf) != NV_OK) {
                return NV_ERROR;
            }
        }
    }
    return NV_OK;
}

int nv_base_cleanup(void)
{
    int i;
    for (i = g_module_count - 1; i >= 0; i--) {
        if (g_modules[i] && g_modules[i]->cleanup) {
            g_modules[i]->cleanup(g_modules[i]->private_data);
        }
    }
    g_module_count = 0;
    return NV_OK;
}

nv_cycle_t *nv_base_create_cycle(nv_conf_t *conf)
{
    nv_cycle_t *cycle;

    cycle = (nv_cycle_t *)calloc(1, sizeof(nv_cycle_t));
    if (!cycle) {
        return NULL;
    }
    cycle->conf = conf;
    cycle->running = 0;
    cycle->module_count = g_module_count;
    memcpy(cycle->modules, g_modules, sizeof(g_modules[0]) * (size_t)g_module_count);
    g_cycle = cycle;
    return cycle;
}

int nv_base_run_cycle(nv_cycle_t *cycle)
{
    if (!cycle) {
        return NV_ERROR;
    }
    cycle->running = 1;
    return NV_OK;
}

void nv_cpuinfo(void)
{
    /* 预留：CPU 信息探测 */
}
