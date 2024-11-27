#include "nv_version.h"

const char* hv_compile_version() {
    static char s_version[16] = {0};
    datetime_t dt = nv_compile_datetime();
    snprintf(s_version, sizeof(s_version), "%d.%d.%d.%d",
        NV_VERSION_MAJOR, dt.year%100, dt.month, dt.day);
    return s_version;
}

