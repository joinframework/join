cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(error.gtest)
install(TARGETS error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(utils.gtest)
install(TARGETS utils.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(filesystem.gtest filesystem_test.cpp)
target_link_libraries(filesystem.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(filesystem.gtest)
install(TARGETS filesystem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(traits.gtest)
install(TARGETS traits.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(variant.gtest)
install(TARGETS variant.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(reactor.gtest)
install(TARGETS reactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(realtimetimer.gtest realtimetimer_test.cpp)
target_link_libraries(realtimetimer.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(realtimetimer.gtest)
install(TARGETS realtimetimer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(monotonictimer.gtest monotonictimer_test.cpp)
target_link_libraries(monotonictimer.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(monotonictimer.gtest)
install(TARGETS monotonictimer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedmem.gtest sharedmem_test.cpp)
target_link_libraries(sharedmem.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(sharedmem.gtest)
install(TARGETS sharedmem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(spsc.gtest spsc_test.cpp)
target_link_libraries(spsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(spsc.gtest)
install(TARGETS spsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mpsc.gtest mpsc_test.cpp)
target_link_libraries(mpsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(mpsc.gtest)
install(TARGETS mpsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mpmc.gtest mpmc_test.cpp)
target_link_libraries(mpmc.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(mpmc.gtest)
install(TARGETS mpmc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(spscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(spscendpoint.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(spscendpoint.gtest)
install(TARGETS spscendpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mpscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpscendpoint.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(mpscendpoint.gtest)
install(TARGETS mpscendpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mpmcendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpmcendpoint.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(mpmcendpoint.gtest)
install(TARGETS mpmcendpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(mutex.gtest)
install(TARGETS mutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(recursivemutex.gtest recursivemutex_test.cpp)
target_link_libraries(recursivemutex.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(recursivemutex.gtest)
install(TARGETS recursivemutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedmutex.gtest sharedmutex_test.cpp)
target_link_libraries(sharedmutex.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(sharedmutex.gtest)
install(TARGETS sharedmutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(condition.gtest condition_test.cpp)
target_link_libraries(condition.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(condition.gtest)
install(TARGETS condition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedcondition.gtest sharedcondition_test.cpp)
target_link_libraries(sharedcondition.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(sharedcondition.gtest)
install(TARGETS sharedcondition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(semaphore.gtest semaphore_test.cpp)
target_link_libraries(semaphore.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(semaphore.gtest)
install(TARGETS semaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedsemaphore.gtest sharedsemaphore_test.cpp)
target_link_libraries(sharedsemaphore.gtest ${JOIN_CORE} GTest::gtest_main rt)
gtest_discover_tests(sharedsemaphore.gtest)
install(TARGETS sharedsemaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(thread.gtest)
install(TARGETS thread.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(threadpool.gtest threadpool_test.cpp)
target_link_libraries(threadpool.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(threadpool.gtest)
install(TARGETS threadpool.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(macaddress.gtest macaddress_test.cpp)
target_link_libraries(macaddress.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(macaddress.gtest)
install(TARGETS macaddress.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(ipaddress.gtest ipaddress_test.cpp)
target_link_libraries(ipaddress.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(ipaddress.gtest)
install(TARGETS ipaddress.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(endpoint.gtest)
install(TARGETS endpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(protocol.gtest)
install(TARGETS protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixdgramsocket.gtest unixdgramsocket_test.cpp)
target_link_libraries(unixdgramsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(unixdgramsocket.gtest)
install(TARGETS unixdgramsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixstreamsocket.gtest unixstreamsocket_test.cpp)
target_link_libraries(unixstreamsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(unixstreamsocket.gtest)
install(TARGETS unixstreamsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(netlinksocket.gtest netlinksocket_test.cpp)
target_link_libraries(netlinksocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(netlinksocket.gtest)
install(TARGETS netlinksocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixstreamacceptor.gtest unixstreamacceptor_test.cpp)
target_link_libraries(unixstreamacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(unixstreamacceptor.gtest)
install(TARGETS unixstreamacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(rawsocket.gtest rawsocket_test.cpp)
target_link_libraries(rawsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(rawsocket.gtest)
install(TARGETS rawsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(udpsocket.gtest udpsocket_test.cpp)
target_link_libraries(udpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(udpsocket.gtest)
install(TARGETS udpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(icmpsocket.gtest icmpsocket_test.cpp)
target_link_libraries(icmpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(icmpsocket.gtest)
install(TARGETS icmpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpsocket.gtest tcpsocket_test.cpp)
target_link_libraries(tcpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tcpsocket.gtest)
install(TARGETS tcpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpsocketstream.gtest tcpsocketstream_test.cpp)
target_link_libraries(tcpsocketstream.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tcpsocketstream.gtest)
install(TARGETS tcpsocketstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpacceptor.gtest tcpacceptor_test.cpp)
target_link_libraries(tcpacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tcpacceptor.gtest)
install(TARGETS tcpacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlserror.gtest tlserror_test.cpp)
target_link_libraries(tlserror.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tlserror.gtest)
install(TARGETS tlserror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlssocket.gtest tlssocket_test.cpp)
target_link_libraries(tlssocket.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tlssocket.gtest)
install(TARGETS tlssocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlssocketstream.gtest tlssocketstream_test.cpp)
target_link_libraries(tlssocketstream.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tlssocketstream.gtest)
install(TARGETS tlssocketstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlsacceptor.gtest tlsacceptor_test.cpp)
target_link_libraries(tlsacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(tlsacceptor.gtest)
install(TARGETS tlsacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${JOIN_CORE} GTest::gtest_main)
gtest_discover_tests(openssl.gtest)
install(TARGETS openssl.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
