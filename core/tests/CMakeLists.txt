cmake_minimum_required(VERSION 3.22.1)

find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(error.gtest)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(utils.gtest)

add_executable(filesystem.gtest filesystem_test.cpp)
target_link_libraries(filesystem.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(filesystem.gtest)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(traits.gtest)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(variant.gtest)

add_executable(stringview.gtest stringview_test.cpp)
target_link_libraries(stringview.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(stringview.gtest)

add_executable(streamview.gtest streamview_test.cpp)
target_link_libraries(streamview.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(streamview.gtest)

add_executable(cache.gtest cache_test.cpp)
target_link_libraries(cache.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(cache.gtest)

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(reactor.gtest)

add_executable(realtimetimer.gtest realtimetimer_test.cpp)
target_link_libraries(realtimetimer.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(realtimetimer.gtest)

add_executable(monotonictimer.gtest monotonictimer_test.cpp)
target_link_libraries(monotonictimer.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(monotonictimer.gtest)

add_executable(sharedmem.gtest sharedmem_test.cpp)
target_link_libraries(sharedmem.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(sharedmem.gtest)

add_executable(spsc.gtest spsc_test.cpp)
target_link_libraries(spsc.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(spsc.gtest)

add_executable(mpsc.gtest mpsc_test.cpp)
target_link_libraries(mpsc.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(mpsc.gtest)

add_executable(mpmc.gtest mpmc_test.cpp)
target_link_libraries(mpmc.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(mpmc.gtest)

add_executable(spscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(spscendpoint.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(spscendpoint.gtest)

add_executable(mpscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpscendpoint.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(mpscendpoint.gtest)

add_executable(mpmcendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpmcendpoint.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(mpmcendpoint.gtest)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(mutex.gtest)

add_executable(recursivemutex.gtest recursivemutex_test.cpp)
target_link_libraries(recursivemutex.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(recursivemutex.gtest)

add_executable(sharedmutex.gtest sharedmutex_test.cpp)
target_link_libraries(sharedmutex.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(sharedmutex.gtest)

add_executable(condition.gtest condition_test.cpp)
target_link_libraries(condition.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(condition.gtest)

add_executable(sharedcondition.gtest sharedcondition_test.cpp)
target_link_libraries(sharedcondition.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(sharedcondition.gtest)

add_executable(semaphore.gtest semaphore_test.cpp)
target_link_libraries(semaphore.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(semaphore.gtest)

add_executable(sharedsemaphore.gtest sharedsemaphore_test.cpp)
target_link_libraries(sharedsemaphore.gtest ${JOIN_CORE} GTest::gtest_main pthread rt)
gtest_discover_tests(sharedsemaphore.gtest)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(thread.gtest)

add_executable(threadpool.gtest threadpool_test.cpp)
target_link_libraries(threadpool.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(threadpool.gtest)

add_executable(macaddress.gtest macaddress_test.cpp)
target_link_libraries(macaddress.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(macaddress.gtest)

add_executable(ipaddress.gtest ipaddress_test.cpp)
target_link_libraries(ipaddress.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(ipaddress.gtest)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(endpoint.gtest)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(protocol.gtest)

add_executable(unixdgramsocket.gtest unixdgramsocket_test.cpp)
target_link_libraries(unixdgramsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(unixdgramsocket.gtest)

add_executable(unixstreamsocket.gtest unixstreamsocket_test.cpp)
target_link_libraries(unixstreamsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(unixstreamsocket.gtest)

add_executable(netlinksocket.gtest netlinksocket_test.cpp)
target_link_libraries(netlinksocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(netlinksocket.gtest)

add_executable(unixstreamacceptor.gtest unixstreamacceptor_test.cpp)
target_link_libraries(unixstreamacceptor.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(unixstreamacceptor.gtest)

add_executable(rawsocket.gtest rawsocket_test.cpp)
target_link_libraries(rawsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(rawsocket.gtest)

add_executable(udpsocket.gtest udpsocket_test.cpp)
target_link_libraries(udpsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(udpsocket.gtest)

add_executable(icmpsocket.gtest icmpsocket_test.cpp)
target_link_libraries(icmpsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(icmpsocket.gtest)

add_executable(tcpsocket.gtest tcpsocket_test.cpp)
target_link_libraries(tcpsocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tcpsocket.gtest)

add_executable(tcpsocketstream.gtest tcpsocketstream_test.cpp)
target_link_libraries(tcpsocketstream.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tcpsocketstream.gtest)

add_executable(tcpacceptor.gtest tcpacceptor_test.cpp)
target_link_libraries(tcpacceptor.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tcpacceptor.gtest)

add_executable(tlserror.gtest tlserror_test.cpp)
target_link_libraries(tlserror.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tlserror.gtest)

add_executable(tlssocket.gtest tlssocket_test.cpp)
target_link_libraries(tlssocket.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tlssocket.gtest)

add_executable(tlssocketstream.gtest tlssocketstream_test.cpp)
target_link_libraries(tlssocketstream.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tlssocketstream.gtest)

add_executable(tlsacceptor.gtest tlsacceptor_test.cpp)
target_link_libraries(tlsacceptor.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(tlsacceptor.gtest)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${JOIN_CORE} GTest::gtest_main pthread)
gtest_discover_tests(openssl.gtest)
