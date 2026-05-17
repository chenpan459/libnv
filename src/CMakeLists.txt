# libnv 核心库

file(GLOB_RECURSE NV_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE NV_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(NV_CJSON_DIR "${CMAKE_SOURCE_DIR}/third_party/cjson")
list(APPEND NV_SOURCES "${NV_CJSON_DIR}/cJSON.c")

set(NV_MXML_DIR "${CMAKE_SOURCE_DIR}/third_party/mxml")
file(GLOB NV_MXML_SOURCES "${NV_MXML_DIR}/mxml-*.c")
list(APPEND NV_SOURCES ${NV_MXML_SOURCES})
# 第三方 mxml 含刻意 fallthrough 的 switch，屏蔽该告警
set_source_files_properties(${NV_MXML_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-Wno-implicit-fallthrough"
)

set(NV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${NV_CJSON_DIR}" "${NV_MXML_DIR}")

# SQLite（可选，内嵌 amalgamation）
if(NV_ENABLE_SQLITE)
    set(NV_SQLITE_DIR "${CMAKE_SOURCE_DIR}/third_party/sqlite")
    list(APPEND NV_SOURCES "${NV_SQLITE_DIR}/sqlite3.c")
    list(APPEND NV_INCLUDE_DIRS "${NV_SQLITE_DIR}")
    set(NV_SQLITE_DEFS
        NV_HAVE_SQLITE=1
        SQLITE_THREADSAFE=1
        SQLITE_OMIT_LOAD_EXTENSION
    )
    # 第三方 amalgamation 含刻意 fallthrough 的 switch，屏蔽该告警
    set_source_files_properties("${NV_SQLITE_DIR}/sqlite3.c" PROPERTIES
        COMPILE_OPTIONS "-Wno-implicit-fallthrough"
    )
    message(STATUS "SQLite enabled (vendored amalgamation)")
else()
    message(STATUS "NV_ENABLE_SQLITE=OFF, excluding nv_sqlite.c")
    list(FILTER NV_SOURCES EXCLUDE REGEX "/nv_sqlite\\.c$")
endif()
foreach(_hdr IN LISTS NV_HEADERS)
    get_filename_component(_dir "${_hdr}" DIRECTORY)
    list(APPEND NV_INCLUDE_DIRS "${_dir}")
endforeach()
list(REMOVE_DUPLICATES NV_INCLUDE_DIRS)

# USB 模块（可选）
if(NV_ENABLE_USB)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBUSB QUIET libusb-1.0)
    endif()
    if(LIBUSB_FOUND)
        message(STATUS "libusb-1.0 found, enabling nv_usb")
    else()
        message(STATUS "libusb-1.0 not found, excluding nv_usb.c")
        list(FILTER NV_SOURCES EXCLUDE REGEX "/nv_usb\\.c$")
    endif()
else()
    message(STATUS "NV_ENABLE_USB=OFF, excluding nv_usb.c")
    list(FILTER NV_SOURCES EXCLUDE REGEX "/nv_usb\\.c$")
endif()

add_library(nv_shared SHARED ${NV_SOURCES})
add_library(nv ALIAS nv_shared)

if(NV_BUILD_STATIC)
    add_library(nv_static STATIC ${NV_SOURCES})
endif()

# 中间文件(.o)由 CMake 生成在 build/src/CMakeFiles/...，不会写入源码目录
set_target_properties(nv_shared PROPERTIES
    OUTPUT_NAME nv
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
set(_nv_lib_targets nv_shared)
if(NV_BUILD_STATIC)
    set_target_properties(nv_static PROPERTIES OUTPUT_NAME nv)
    list(APPEND _nv_lib_targets nv_static)
endif()

foreach(_tgt IN LISTS _nv_lib_targets)
    target_include_directories(${_tgt}
        PUBLIC
            $<BUILD_INTERFACE:${NV_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(${_tgt} PRIVATE _GNU_SOURCE)
    if(NV_COMPILE_DEFS)
        target_compile_definitions(${_tgt} PRIVATE ${NV_COMPILE_DEFS})
    endif()
    if(NV_ENABLE_SQLITE AND NV_SQLITE_DEFS)
        target_compile_definitions(${_tgt} PRIVATE ${NV_SQLITE_DEFS})
    endif()
    target_compile_options(${_tgt} PRIVATE -Wall -Wextra)
    target_link_libraries(${_tgt}
        PUBLIC
            Threads::Threads
            m
            dl
            rt
    )
    if(LIBUSB_FOUND)
        target_link_libraries(${_tgt} PUBLIC ${LIBUSB_LIBRARIES})
        target_include_directories(${_tgt} PUBLIC ${LIBUSB_INCLUDE_DIRS})
    endif()
endforeach()
