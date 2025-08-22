/************************************************
 * @文件名: event_loop_demo.c
 * @功能: 事件循环使用示例
 * @作者: chenpan
 * @日期: 2024-11-04
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "../src/event/nv_loop.h"
#include "../src/event/nv_event.h"

/* 全局变量 */
static nv_loop_t main_loop;
static int running = 1;

/* 信号处理函数 */
void signal_handler(int sig) {
    printf("Received signal %d, stopping event loop...\n", sig);
    running = 0;
    nv_loop_stop(&main_loop);
}

/* 定时器事件处理函数 */
void timer_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_ext_t *event = (nv_event_ext_t *)ev;
    printf("Timer event triggered! Count: %lu\n", event->trigger_count);
    
    /* 重新设置定时器（周期性定时器） */
    if (event->timeout_ms > 0) {
        nv_loop_add_timer(loop, event, event->timeout_ms);
    }
}

/* 空闲事件处理函数 */
void idle_handler(nv_loop_t *loop, void *ev, void *data) {
    static int idle_count = 0;
    idle_count++;
    
    if (idle_count % 100 == 0) {
        printf("Idle event processed %d times\n", idle_count);
    }
}

/* 主函数 */
int main() {
    printf("NV Event Loop Demo\n");
    printf("==================\n");
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* 创建事件循环配置 */
    nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
    config.max_events = 128;
    config.timeout_ms = 100;
    config.enable_timers = 1;
    config.enable_idle = 1;
    config.enable_signals = 1;
    
    /* 初始化事件循环 */
    if (nv_loop_init(&main_loop, &config) != 0) {
        perror("Failed to initialize event loop");
        return EXIT_FAILURE;
    }
    
    printf("Event loop initialized successfully\n");
    
    /* 创建定时器事件 */
    nv_event_ext_t timer_event;
    NV_EVENT_INIT(&timer_event, timer_handler, NULL);
    nv_event_set_timeout(&timer_event, 1000); /* 1秒定时器 */
    
    if (nv_loop_add_timer(&main_loop, &timer_event, 1000) != 0) {
        perror("Failed to add timer event");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("Timer event added (1 second interval)\n");
    
    /* 创建空闲事件 */
    nv_event_ext_t idle_event;
    NV_EVENT_INIT(&idle_event, idle_handler, NULL);
    
    if (nv_loop_add_idle(&main_loop, &idle_event) != 0) {
        perror("Failed to add idle event");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("Idle event added\n");
    
    /* 运行事件循环 */
    printf("Starting event loop...\n");
    printf("Press Ctrl+C to stop\n");
    
    if (nv_loop_run(&main_loop) != 0) {
        perror("Event loop run failed");
    }
    
    printf("Event loop stopped\n");
    
    /* 清理资源 */
    nv_event_cleanup(&timer_event);
    nv_event_cleanup(&idle_event);
    nv_loop_cleanup(&main_loop);
    
    printf("Cleanup completed\n");
    
    return EXIT_SUCCESS;
}
