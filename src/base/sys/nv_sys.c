#include "nv_sys.h"

//seconds单位秒
void nv_sleep(unsigned int seconds) {
    sleep(seconds);
}

// milliseconds 单位毫秒
void nv_msleep(unsigned int milliseconds) {
    struct timespec req;
    req.tv_sec = milliseconds / 1000; // 秒
    req.tv_nsec = (milliseconds % 1000) * 1000000; // 纳秒
    nanosleep(&req, NULL);
}

// nanoseconds 单位纳秒
void nv_nsleep(unsigned int nanoseconds) {
    struct timespec req;
    req.tv_sec = nanoseconds / 1000000000; // 秒
    req.tv_nsec = nanoseconds % 1000000000; // 纳秒
    nanosleep(&req, NULL);
}

