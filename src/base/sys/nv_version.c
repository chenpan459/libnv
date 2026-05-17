#include "nv_version.h"

const char *nv_version_string(void)
{
    return NV_VERSION_STRING;
}

const char *nv_build_time_string(void)
{
    return NV_BUILD_TIME;
}

const char *hv_compile_version(void)
{
    return nv_version_string();
}
