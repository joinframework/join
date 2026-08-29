cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME error.gtest COMMAND error.gtest)
install(TARGETS error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME utils.gtest COMMAND utils.gtest)
install(TARGETS utils.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(filesystem.gtest filesystem_test.cpp)
target_link_libraries(filesystem.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME filesystem.gtest COMMAND filesystem.gtest)
install(TARGETS filesystem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME traits.gtest COMMAND traits.gtest)
install(TARGETS traits.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME variant.gtest COMMAND variant.gtest)
install(TARGETS variant.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(backoff.gtest backoff_test.cpp)
target_link_libraries(backoff.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME backoff.gtest COMMAND backoff.gtest)
install(TARGETS backoff.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME reactor.gtest COMMAND reactor.gtest)
install(TARGETS reactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(io_operation.gtest io_operation_test.cpp)
target_link_libraries(io_operation.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME io_operation.gtest COMMAND io_operation.gtest)
install(TARGETS io_operation.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

if(JOIN_ENABLE_IO_URING)
    add_executable(io_policy.gtest io_policy_test.cpp)
    target_link_libraries(io_policy.gtest ${JOIN_CORE} GTest::gtest_main)
    add_test(NAME io_policy.gtest COMMAND io_policy.gtest)
    install(TARGETS io_policy.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
endif()

add_executable(proactor.gtest proactor_test.cpp)
target_link_libraries(proactor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME proactor.gtest COMMAND proactor.gtest)
install(TARGETS proactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

if(JOIN_ENABLE_IO_URING)
    add_executable(hybrid_proactor.gtest hybrid_proactor_test.cpp)
    target_link_libraries(hybrid_proactor.gtest ${JOIN_CORE} GTest::gtest_main)
    add_test(NAME hybrid_proactor.gtest COMMAND hybrid_proactor.gtest)
    install(TARGETS hybrid_proactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

    add_executable(sqpoll_proactor.gtest sqpoll_proactor_test.cpp)
    target_link_libraries(sqpoll_proactor.gtest ${JOIN_CORE} GTest::gtest_main)
    add_test(NAME sqpoll_proactor.gtest COMMAND sqpoll_proactor.gtest)
    install(TARGETS sqpoll_proactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
endif()

add_executable(realtime_timer.gtest realtime_timer_test.cpp)
target_link_libraries(realtime_timer.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME realtime_timer.gtest COMMAND realtime_timer.gtest)
install(TARGETS realtime_timer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(monotonic_timer.gtest monotonic_timer_test.cpp)
target_link_libraries(monotonic_timer.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME monotonic_timer.gtest COMMAND monotonic_timer.gtest)
install(TARGETS monotonic_timer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(monotonic_stats.gtest monotonic_stats_test.cpp)
target_link_libraries(monotonic_stats.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME monotonic_stats.gtest COMMAND monotonic_stats.gtest)
install(TARGETS monotonic_stats.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(monotonicraw_stats.gtest monotonicraw_stats_test.cpp)
target_link_libraries(monotonicraw_stats.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME monotonicraw_stats.gtest COMMAND monotonicraw_stats.gtest)
install(TARGETS monotonicraw_stats.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(rdtsc_stats.gtest rdtsc_stats_test.cpp)
target_link_libraries(rdtsc_stats.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME rdtsc_stats.gtest COMMAND rdtsc_stats.gtest)
install(TARGETS rdtsc_stats.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(local_mem.gtest local_mem_test.cpp)
target_link_libraries(local_mem.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME local_mem.gtest COMMAND local_mem.gtest)
install(TARGETS local_mem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(local_alloc.gtest local_alloc_test.cpp)
target_link_libraries(local_alloc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME local_alloc.gtest COMMAND local_alloc.gtest)
install(TARGETS local_alloc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(local_spsc.gtest local_spsc_test.cpp)
target_link_libraries(local_spsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME local_spsc.gtest COMMAND local_spsc.gtest)
install(TARGETS local_spsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(local_mpsc.gtest local_mpsc_test.cpp)
target_link_libraries(local_mpsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME local_mpsc.gtest COMMAND local_mpsc.gtest)
install(TARGETS local_mpsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(local_mpmc.gtest local_mpmc_test.cpp)
target_link_libraries(local_mpmc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME local_mpmc.gtest COMMAND local_mpmc.gtest)
install(TARGETS local_mpmc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shm_mem.gtest shm_mem_test.cpp)
target_link_libraries(shm_mem.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shm_mem.gtest COMMAND shm_mem.gtest)
install(TARGETS shm_mem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shm_alloc.gtest shm_alloc_test.cpp)
target_link_libraries(shm_alloc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shm_alloc.gtest COMMAND shm_alloc.gtest)
install(TARGETS shm_alloc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shm_spsc.gtest shm_spsc_test.cpp)
target_link_libraries(shm_spsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shm_spsc.gtest COMMAND shm_spsc.gtest)
install(TARGETS shm_spsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shm_mpsc.gtest shm_mpsc_test.cpp)
target_link_libraries(shm_mpsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shm_mpsc.gtest COMMAND shm_mpsc.gtest)
install(TARGETS shm_mpsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shm_mpmc.gtest shm_mpmc_test.cpp)
target_link_libraries(shm_mpmc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shm_mpmc.gtest COMMAND shm_mpmc.gtest)
install(TARGETS shm_mpmc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME mutex.gtest COMMAND mutex.gtest)
install(TARGETS mutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(recursive_mutex.gtest recursive_mutex_test.cpp)
target_link_libraries(recursive_mutex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME recursive_mutex.gtest COMMAND recursive_mutex.gtest)
install(TARGETS recursive_mutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shared_mutex.gtest shared_mutex_test.cpp)
target_link_libraries(shared_mutex.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shared_mutex.gtest COMMAND shared_mutex.gtest)
install(TARGETS shared_mutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(condition.gtest condition_test.cpp)
target_link_libraries(condition.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME condition.gtest COMMAND condition.gtest)
install(TARGETS condition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shared_condition.gtest shared_condition_test.cpp)
target_link_libraries(shared_condition.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shared_condition.gtest COMMAND shared_condition.gtest)
install(TARGETS shared_condition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(semaphore.gtest semaphore_test.cpp)
target_link_libraries(semaphore.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME semaphore.gtest COMMAND semaphore.gtest)
install(TARGETS semaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shared_semaphore.gtest shared_semaphore_test.cpp)
target_link_libraries(shared_semaphore.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shared_semaphore.gtest COMMAND shared_semaphore.gtest)
install(TARGETS shared_semaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(futex.gtest futex_test.cpp)
target_link_libraries(futex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME futex.gtest COMMAND futex.gtest)
install(TARGETS futex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shared_futex.gtest shared_futex_test.cpp)
target_link_libraries(shared_futex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME shared_futex.gtest COMMAND shared_futex.gtest)
install(TARGETS shared_futex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(function.gtest function_test.cpp)
target_link_libraries(function.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME function.gtest COMMAND function.gtest)
install(TARGETS function.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME thread.gtest COMMAND thread.gtest)
install(TARGETS thread.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(thread_pool.gtest thread_pool_test.cpp)
target_link_libraries(thread_pool.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME thread_pool.gtest COMMAND thread_pool.gtest)
install(TARGETS thread_pool.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mac_address.gtest mac_address_test.cpp)
target_link_libraries(mac_address.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME mac_address.gtest COMMAND mac_address.gtest)
install(TARGETS mac_address.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(ip_address.gtest ip_address_test.cpp)
target_link_libraries(ip_address.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME ip_address.gtest COMMAND ip_address.gtest)
install(TARGETS ip_address.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME endpoint.gtest COMMAND endpoint.gtest)
install(TARGETS endpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME protocol.gtest COMMAND protocol.gtest)
install(TARGETS protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_datagram_socket.gtest unix_datagram_socket_test.cpp)
target_link_libraries(unix_datagram_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_datagram_socket.gtest COMMAND unix_datagram_socket.gtest)
install(TARGETS unix_datagram_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_stream_socket.gtest unix_stream_socket_test.cpp)
target_link_libraries(unix_stream_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_stream_socket.gtest COMMAND unix_stream_socket.gtest)
install(TARGETS unix_stream_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_stream_acceptor.gtest unix_stream_acceptor_test.cpp)
target_link_libraries(unix_stream_acceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_stream_acceptor.gtest COMMAND unix_stream_acceptor.gtest)
install(TARGETS unix_stream_acceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(raw_socket.gtest raw_socket_test.cpp)
target_link_libraries(raw_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME raw_socket.gtest COMMAND raw_socket.gtest)
install(TARGETS raw_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(udp_socket.gtest udp_socket_test.cpp)
target_link_libraries(udp_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME udp_socket.gtest COMMAND udp_socket.gtest)
install(TARGETS udp_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(icmp_socket.gtest icmp_socket_test.cpp)
target_link_libraries(icmp_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME icmp_socket.gtest COMMAND icmp_socket.gtest)
install(TARGETS icmp_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcp_socket.gtest tcp_socket_test.cpp)
target_link_libraries(tcp_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcp_socket.gtest COMMAND tcp_socket.gtest)
install(TARGETS tcp_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcp_socket_stream.gtest tcp_socket_stream_test.cpp)
target_link_libraries(tcp_socket_stream.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcp_socket_stream.gtest COMMAND tcp_socket_stream.gtest)
install(TARGETS tcp_socket_stream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcp_acceptor.gtest tcp_acceptor_test.cpp)
target_link_libraries(tcp_acceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcp_acceptor.gtest COMMAND tcp_acceptor.gtest)
install(TARGETS tcp_acceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_async_datagram_socket.gtest unix_async_datagram_socket_test.cpp)
target_link_libraries(unix_async_datagram_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_async_datagram_socket.gtest COMMAND unix_async_datagram_socket.gtest)
install(TARGETS unix_async_datagram_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_async_stream_socket.gtest unix_async_stream_socket_test.cpp)
target_link_libraries(unix_async_stream_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_async_stream_socket.gtest COMMAND unix_async_stream_socket.gtest)
install(TARGETS unix_async_stream_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unix_async_acceptor.gtest unix_async_acceptor_test.cpp)
target_link_libraries(unix_async_acceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unix_async_acceptor.gtest COMMAND unix_async_acceptor.gtest)
install(TARGETS unix_async_acceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(raw_async_socket.gtest raw_async_socket_test.cpp)
target_link_libraries(raw_async_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME raw_async_socket.gtest COMMAND raw_async_socket.gtest)
install(TARGETS raw_async_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(udp_async_datagram_socket.gtest udp_async_datagram_socket_test.cpp)
target_link_libraries(udp_async_datagram_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME udp_async_datagram_socket.gtest COMMAND udp_async_datagram_socket.gtest)
install(TARGETS udp_async_datagram_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(icmp_async_datagram_socket.gtest icmp_async_datagram_socket_test.cpp)
target_link_libraries(icmp_async_datagram_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME icmp_async_datagram_socket.gtest COMMAND icmp_async_datagram_socket.gtest)
install(TARGETS icmp_async_datagram_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcp_async_stream_socket.gtest tcp_async_stream_socket_test.cpp)
target_link_libraries(tcp_async_stream_socket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcp_async_stream_socket.gtest COMMAND tcp_async_stream_socket.gtest)
install(TARGETS tcp_async_stream_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcp_async_acceptor.gtest tcp_async_acceptor_test.cpp)
target_link_libraries(tcp_async_acceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcp_async_acceptor.gtest COMMAND tcp_async_acceptor.gtest)
install(TARGETS tcp_async_acceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(cpu.gtest cpu_test.cpp)
target_link_libraries(cpu.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME cpu.gtest COMMAND cpu.gtest)
install(TARGETS cpu.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
