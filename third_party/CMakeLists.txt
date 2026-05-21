# third_party 各组件独立编译为静态库 / 动态库（OBJECT 复用，避免重复编译）

set(NV_THIRD_PARTY_LIBS "")
set(NV_THIRD_PARTY_LIBS_SHARED "")

# --- cJSON -> libcjson.a / libcjson.so ---
set(NV_CJSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cjson")
add_library(cjson_obj OBJECT "${NV_CJSON_DIR}/cJSON.c")
set_target_properties(cjson_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(cjson_obj PUBLIC "${NV_CJSON_DIR}")

add_library(cjson_static STATIC $<TARGET_OBJECTS:cjson_obj>)
add_library(cjson_shared SHARED $<TARGET_OBJECTS:cjson_obj>)
target_include_directories(cjson_static PUBLIC "${NV_CJSON_DIR}")
target_include_directories(cjson_shared PUBLIC "${NV_CJSON_DIR}")
target_compile_options(cjson_static PRIVATE -Wall -Wextra)
target_compile_options(cjson_shared PRIVATE -Wall -Wextra)
set_target_properties(cjson_static PROPERTIES OUTPUT_NAME cjson)
set_target_properties(cjson_shared PROPERTIES
    OUTPUT_NAME cjson
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
list(APPEND NV_THIRD_PARTY_LIBS cjson_static)
list(APPEND NV_THIRD_PARTY_LIBS_SHARED cjson_shared)

# --- Mini-XML -> libmxml.a / libmxml.so ---
set(NV_MXML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mxml")
file(GLOB NV_MXML_SOURCES "${NV_MXML_DIR}/mxml-*.c")
set_source_files_properties(${NV_MXML_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-Wno-implicit-fallthrough"
)
add_library(mxml_obj OBJECT ${NV_MXML_SOURCES})
set_target_properties(mxml_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(mxml_obj PRIVATE _DEFAULT_SOURCE)
target_include_directories(mxml_obj PUBLIC "${NV_MXML_DIR}")

add_library(mxml_static STATIC $<TARGET_OBJECTS:mxml_obj>)
add_library(mxml_shared SHARED $<TARGET_OBJECTS:mxml_obj>)
target_include_directories(mxml_static PUBLIC "${NV_MXML_DIR}")
target_include_directories(mxml_shared PUBLIC "${NV_MXML_DIR}")
target_compile_options(mxml_static PRIVATE -Wall -Wextra)
target_compile_options(mxml_shared PRIVATE -Wall -Wextra)
set_target_properties(mxml_static PROPERTIES OUTPUT_NAME mxml)
set_target_properties(mxml_shared PROPERTIES
    OUTPUT_NAME mxml
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
list(APPEND NV_THIRD_PARTY_LIBS mxml_static)
list(APPEND NV_THIRD_PARTY_LIBS_SHARED mxml_shared)

# --- SQLite -> libsqlite.a / libsqlite.so（可选）---
if(NV_ENABLE_SQLITE)
    set(NV_SQLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sqlite")
    set_source_files_properties("${NV_SQLITE_DIR}/sqlite3.c" PROPERTIES
        COMPILE_OPTIONS "-Wno-implicit-fallthrough"
    )
    add_library(sqlite_obj OBJECT "${NV_SQLITE_DIR}/sqlite3.c")
    set_target_properties(sqlite_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(sqlite_obj PUBLIC "${NV_SQLITE_DIR}")

    add_library(sqlite_static STATIC $<TARGET_OBJECTS:sqlite_obj>)
    add_library(sqlite_shared SHARED $<TARGET_OBJECTS:sqlite_obj>)
    target_include_directories(sqlite_static PUBLIC "${NV_SQLITE_DIR}")
    target_include_directories(sqlite_shared PUBLIC "${NV_SQLITE_DIR}")
    target_compile_definitions(sqlite_static PUBLIC
        NV_HAVE_SQLITE=1
        SQLITE_THREADSAFE=1
        SQLITE_OMIT_LOAD_EXTENSION
    )
    target_compile_definitions(sqlite_shared PUBLIC
        NV_HAVE_SQLITE=1
        SQLITE_THREADSAFE=1
        SQLITE_OMIT_LOAD_EXTENSION
    )
    target_compile_options(sqlite_static PRIVATE -Wall -Wextra)
    target_compile_options(sqlite_shared PRIVATE -Wall -Wextra)
    set_target_properties(sqlite_static PROPERTIES OUTPUT_NAME sqlite)
    set_target_properties(sqlite_shared PROPERTIES
        OUTPUT_NAME sqlite
        VERSION   ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    list(APPEND NV_THIRD_PARTY_LIBS sqlite_static)
    list(APPEND NV_THIRD_PARTY_LIBS_SHARED sqlite_shared)
    message(STATUS "third_party: sqlite -> libsqlite.a / libsqlite.so")
endif()

message(STATUS "third_party: cjson -> libcjson.a / libcjson.so")
message(STATUS "third_party: mxml  -> libmxml.a / libmxml.so")

set(NV_THIRD_PARTY_LIBS "${NV_THIRD_PARTY_LIBS}" PARENT_SCOPE)
set(NV_THIRD_PARTY_LIBS_SHARED "${NV_THIRD_PARTY_LIBS_SHARED}" PARENT_SCOPE)
set(NV_CJSON_DIR "${NV_CJSON_DIR}" PARENT_SCOPE)
set(NV_MXML_DIR "${NV_MXML_DIR}" PARENT_SCOPE)
if(NV_ENABLE_SQLITE)
    set(NV_SQLITE_DIR "${NV_SQLITE_DIR}" PARENT_SCOPE)
endif()
