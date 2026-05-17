set(_NV_EXAMPLES
    event_loop_demo
    event_loop_advanced_demo
    network_demo
    memory_pool_demo
    simple_server
)

foreach(_name IN LISTS _NV_EXAMPLES)
    add_executable(${_name} ${_name}.c)
    target_link_libraries(${_name} PRIVATE nv_shared)
endforeach()
