# libnv 核心库

file(GLOB_RECURSE NV_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB_RECURSE NV_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(NV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
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
