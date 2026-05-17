function(nv_add_unit_test _name)
    add_executable(${_name} ${ARGN})
    target_link_libraries(${_name} PRIVATE nv_static)
    add_test(NAME ${_name} COMMAND ${_name})
endfunction()

enable_testing()

nv_add_unit_test(test_nv_secure test_nv_secure.c)
nv_add_unit_test(test_nv_log test_nv_log.c)
nv_add_unit_test(test_nv_json test_nv_json.c)
nv_add_unit_test(test_nv_xml test_nv_xml.c)
nv_add_unit_test(test_nv_sqlite test_nv_sqlite.c)
