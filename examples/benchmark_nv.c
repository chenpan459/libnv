/************************************************
 * @文件名: nv_socket_types.h
 * @功能: Socket库类型定义头文件
 * @作者: chenpan
 * @日期: 2024-11-04
 * 
 * @修改记录:
 * 2024-11-04 - 创建文件，定义基本数据类型
 ***********************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <nv_log.h>

int main(int argc, char **argv) 
{
   #if 0
    // 初始化日志系统
    if (nv_log_init("app.log") != 0) {
        printf("Failed to initialize NV log system\n");
        return -1;
    }
	#endif

    // 设置日志级别（可选，默认为DEBUG）
    nv_log_set_level(NV_LOG_DEBUG);
    
    // 使用日志
    nv_log_debug("这是一条调试信息");
    nv_log_info("系统启动，版本号: %s", "1.0.0");
    nv_log_warn("内存使用率: %d%%", 85);
    nv_log_error("文件打开失败: %s", "config.json");
   // nv_log_fatal("系统崩溃");
    
    // 关闭日志系统
   // nv_log_close();


   return 0;
}	