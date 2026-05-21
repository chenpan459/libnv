# 嵌入式发行预设：更小体积、更少依赖
# 用法: cmake -S . -B build -DNV_EMBEDDED_PROFILE=ON ...

if(NV_EMBEDDED_PROFILE)
    set(NV_ENABLE_SQLITE OFF CACHE BOOL "Embedded: no SQLite" FORCE)
    set(NV_ENABLE_USB OFF CACHE BOOL "Embedded: no USB" FORCE)
    set(NV_BUILD_EXAMPLES OFF CACHE BOOL "Embedded: no examples" FORCE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
        string(APPEND CMAKE_C_FLAGS_RELEASE " -Os")
    endif()
    message(STATUS "NV_EMBEDDED_PROFILE=ON (SQLite/USB off, -Os on Release)")
endif()
