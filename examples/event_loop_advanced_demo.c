/************************************************
 * @文件名: event_loop_advanced_demo.c
 * @功能: 高级事件循环使用示例
 * @作者: chenpan
 * @日期: 2024-11-04
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../src/event/nv_loop.h"
#include "../src/event/nv_event.h"

/* 全局变量 */
static nv_loop_t main_loop;
static int running = 1;
static int timer_count = 0;
static int idle_count = 0;

/* 信号处理函数 */
void signal_handler(int sig) {
    printf("Received signal %d, stopping event loop...\n", sig);
    running = 0;
    nv_loop_stop(&main_loop);
}

/* 定时器事件处理函数 */
void timer_handler(nv_loop_t *loop, void *ev, void *data) {
    nv_event_ext_t *event = (nv_event_ext_t *)ev;
    timer_count++;
    
    printf("Timer event triggered! Count: %d, Total: %d\n", 
           event->trigger_count, timer_count);
    
    /* 每5次触发后停止定时器 */
    if (timer_count >= 5) {
        printf("Stopping timer after 5 triggers\n");
        nv_loop_del_timer(loop, event);
        return;
    }
    
    /* 重新设置定时器（周期性定时器） */
    if (event->timeout_ms > 0) {
        nv_loop_add_timer(loop, event, event->timeout_ms);
    }
}

/* 空闲事件处理函数 */
void idle_handler(nv_loop_t *loop, void *ev, void *data) {
    idle_count++;
    
    if (idle_count % 50 == 0) {
        printf("Idle event processed %d times\n", idle_count);
    }
    
    /* 模拟一些后台工作 */
    usleep(1000); /* 1毫秒 */
}

/* 统计信息打印函数 */
void print_stats(nv_loop_t *loop) {
    nv_loop_stats_t stats;
    nv_loop_get_stats(loop, &stats);
    
    printf("\n=== Event Loop Statistics ===\n");
    printf("Events processed: %lu\n", stats.events_processed);
    printf("Timers processed: %lu\n", stats.timers_processed);
    printf("Signals processed: %lu\n", stats.signals_processed);
    printf("Idle events processed: %lu\n", stats.idle_events_processed);
    printf("Total events: %lu\n", stats.total_events);
    printf("Active events: %lu\n", stats.active_events);
    printf("Pending events: %lu\n", stats.pending_events);
    printf("=============================\n\n");
}

/* 配置更新演示 */
void demonstrate_config_update(nv_loop_t *loop) {
    printf("Demonstrating configuration update...\n");
    
    nv_loop_config_t current_config;
    if (nv_loop_get_config(loop, &current_config) == 0) {
        printf("Current config - Max events: %d, Timeout: %d ms\n",
               current_config.max_events, current_config.timeout_ms);
        
        /* 更新配置 */
        current_config.timeout_ms = 500;
        if (nv_loop_set_config(loop, &current_config) == 0) {
            printf("Configuration updated successfully\n");
        }
    }
}

/* 事件循环控制演示 */
void demonstrate_loop_control(nv_loop_t *loop) {
    printf("Demonstrating loop control...\n");
    
    /* 暂停事件循环 */
    printf("Pausing event loop...\n");
    nv_loop_pause(loop);
    sleep(1);
    
    /* 恢复事件循环 */
    printf("Resuming event loop...\n");
    nv_loop_resume(loop);
}

/* 主函数 */
int main() {
    printf("NV Advanced Event Loop Demo\n");
    printf("============================\n");
    
    /* 设置信号处理 */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* 创建事件循环配置 */
    nv_loop_config_t config = NV_LOOP_CONFIG_DEFAULT;
    config.max_events = 256;
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
    nv_event_set_priority(&timer_event, NV_EVENT_PRIO_HIGH);
    
    if (nv_loop_add_timer(&main_loop, &timer_event, 1000) != 0) {
        perror("Failed to add timer event");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("High priority timer event added (1 second interval)\n");
    
    /* 创建空闲事件 */
    nv_event_ext_t idle_event;
    NV_EVENT_INIT(&idle_event, idle_handler, NULL);
    nv_event_set_priority(&idle_event, NV_EVENT_PRIO_LOW);
    
    if (nv_loop_add_idle(&main_loop, &idle_event) != 0) {
        perror("Failed to add idle event");
        nv_loop_cleanup(&main_loop);
        return EXIT_FAILURE;
    }
    
    printf("Low priority idle event added\n");
    
    /* 演示配置更新 */
    demonstrate_config_update(&main_loop);
    
    /* 运行事件循环 */
    printf("Starting event loop...\n");
    printf("Press Ctrl+C to stop\n");
    
    /* 在另一个线程中演示循环控制 */
    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程：等待一段时间后演示循环控制 */
        sleep(3);
        demonstrate_loop_control(&main_loop);
        exit(0);
    }
    
    if (nv_loop_run(&main_loop) != 0) {
        perror("Event loop run failed");
    }
    
    printf("Event loop stopped\n");
    
    /* 打印最终统计信息 */
    print_stats(&main_loop);
    
    /* 清理资源 */
    nv_event_cleanup(&timer_event);
    nv_event_cleanup(&idle_event);
    nv_loop_cleanup(&main_loop);
    
    printf("Cleanup completed\n");
    
    return EXIT_SUCCESS;
}
