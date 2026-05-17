# 单元测试（默认）
add_subdirectory(unit)

# 历史网络/ARP 示例（默认关闭，部分待修复）
if(NV_BUILD_TESTS_LEGACY)
    function(nv_add_test_exe _name)
        add_executable(${_name} ${ARGN})
        target_link_libraries(${_name} PRIVATE nv_shared)
    endfunction()

    nv_add_test_exe(benchmark_nv ex0/benchmark_nv.c)
    nv_add_test_exe(nv_all_test ex1/nv_all_test.c)
    nv_add_test_exe(nv_tcp_cli ex2/nv_tcp_cli.c)
    nv_add_test_exe(nv_tcp_srv ex2/nv_tcp_srv.c)
    nv_add_test_exe(nv_udp_cli ex3/nv_udp_cli.c)
    nv_add_test_exe(nv_udp_srv ex3/nv_udp_srv.c)
    nv_add_test_exe(multicast_ipv4 ex3/multicast_ipv4.c)
    nv_add_test_exe(multicast_ipv6 ex3/multicast_ipv6.c)
    nv_add_test_exe(nv_net ex4/nv_net.c)

    set(_ARP_TESTS
        gratuitous_arp gratuitous_na gratuitous_nav6 arp_broadcast
        arp_aging arp_aging_message ndp_aging ndp_aging_message
    )
    foreach(_t IN LISTS _ARP_TESTS)
        nv_add_test_exe(${_t} arp/${_t}.c)
    endforeach()
endif()
