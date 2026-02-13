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

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME reactor.gtest COMMAND reactor.gtest)
install(TARGETS reactor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(realtimetimer.gtest realtimetimer_test.cpp)
target_link_libraries(realtimetimer.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME realtimetimer.gtest COMMAND realtimetimer.gtest)
install(TARGETS realtimetimer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(monotonictimer.gtest monotonictimer_test.cpp)
target_link_libraries(monotonictimer.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME monotonictimer.gtest COMMAND monotonictimer.gtest)
install(TARGETS monotonictimer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(localmem.gtest localmem_test.cpp)
target_link_libraries(localmem.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME localmem.gtest COMMAND localmem.gtest)
install(TARGETS localmem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(localspsc.gtest localspsc_test.cpp)
target_link_libraries(localspsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME localspsc.gtest COMMAND localspsc.gtest)
install(TARGETS localspsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(localmpsc.gtest localmpsc_test.cpp)
target_link_libraries(localmpsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME localmpsc.gtest COMMAND localmpsc.gtest)
install(TARGETS localmpsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(localmpmc.gtest localmpmc_test.cpp)
target_link_libraries(localmpmc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME localmpmc.gtest COMMAND localmpmc.gtest)
install(TARGETS localmpmc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shmmem.gtest shmmem_test.cpp)
target_link_libraries(shmmem.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shmmem.gtest COMMAND shmmem.gtest)
install(TARGETS shmmem.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shmspsc.gtest shmspsc_test.cpp)
target_link_libraries(shmspsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shmspsc.gtest COMMAND shmspsc.gtest)
install(TARGETS shmspsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shmmpsc.gtest shmmpsc_test.cpp)
target_link_libraries(shmmpsc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shmmpsc.gtest COMMAND shmmpsc.gtest)
install(TARGETS shmmpsc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(shmmpmc.gtest shmmpmc_test.cpp)
target_link_libraries(shmmpmc.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME shmmpmc.gtest COMMAND shmmpmc.gtest)
install(TARGETS shmmpmc.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME mutex.gtest COMMAND mutex.gtest)
install(TARGETS mutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(recursivemutex.gtest recursivemutex_test.cpp)
target_link_libraries(recursivemutex.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME recursivemutex.gtest COMMAND recursivemutex.gtest)
install(TARGETS recursivemutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedmutex.gtest sharedmutex_test.cpp)
target_link_libraries(sharedmutex.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME sharedmutex.gtest COMMAND sharedmutex.gtest)
install(TARGETS sharedmutex.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(condition.gtest condition_test.cpp)
target_link_libraries(condition.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME condition.gtest COMMAND condition.gtest)
install(TARGETS condition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedcondition.gtest sharedcondition_test.cpp)
target_link_libraries(sharedcondition.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME sharedcondition.gtest COMMAND sharedcondition.gtest)
install(TARGETS sharedcondition.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(semaphore.gtest semaphore_test.cpp)
target_link_libraries(semaphore.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME semaphore.gtest COMMAND semaphore.gtest)
install(TARGETS semaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sharedsemaphore.gtest sharedsemaphore_test.cpp)
target_link_libraries(sharedsemaphore.gtest ${JOIN_CORE} GTest::gtest_main rt)
add_test(NAME sharedsemaphore.gtest COMMAND sharedsemaphore.gtest)
install(TARGETS sharedsemaphore.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME thread.gtest COMMAND thread.gtest)
install(TARGETS thread.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(threadpool.gtest threadpool_test.cpp)
target_link_libraries(threadpool.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME threadpool.gtest COMMAND threadpool.gtest)
install(TARGETS threadpool.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(macaddress.gtest macaddress_test.cpp)
target_link_libraries(macaddress.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME macaddress.gtest COMMAND macaddress.gtest)
install(TARGETS macaddress.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(ipaddress.gtest ipaddress_test.cpp)
target_link_libraries(ipaddress.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME ipaddress.gtest COMMAND ipaddress.gtest)
install(TARGETS ipaddress.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME endpoint.gtest COMMAND endpoint.gtest)
install(TARGETS endpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME protocol.gtest COMMAND protocol.gtest)
install(TARGETS protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixdgramsocket.gtest unixdgramsocket_test.cpp)
target_link_libraries(unixdgramsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unixdgramsocket.gtest COMMAND unixdgramsocket.gtest)
install(TARGETS unixdgramsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixstreamsocket.gtest unixstreamsocket_test.cpp)
target_link_libraries(unixstreamsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unixstreamsocket.gtest COMMAND unixstreamsocket.gtest)
install(TARGETS unixstreamsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(netlinksocket.gtest netlinksocket_test.cpp)
target_link_libraries(netlinksocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME netlinksocket.gtest COMMAND netlinksocket.gtest)
install(TARGETS netlinksocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(unixstreamacceptor.gtest unixstreamacceptor_test.cpp)
target_link_libraries(unixstreamacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME unixstreamacceptor.gtest COMMAND unixstreamacceptor.gtest)
install(TARGETS unixstreamacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(rawsocket.gtest rawsocket_test.cpp)
target_link_libraries(rawsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME rawsocket.gtest COMMAND rawsocket.gtest)
install(TARGETS rawsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(udpsocket.gtest udpsocket_test.cpp)
target_link_libraries(udpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME udpsocket.gtest COMMAND udpsocket.gtest)
install(TARGETS udpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(icmpsocket.gtest icmpsocket_test.cpp)
target_link_libraries(icmpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME icmpsocket.gtest COMMAND icmpsocket.gtest)
install(TARGETS icmpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpsocket.gtest tcpsocket_test.cpp)
target_link_libraries(tcpsocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcpsocket.gtest COMMAND tcpsocket.gtest)
install(TARGETS tcpsocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpsocketstream.gtest tcpsocketstream_test.cpp)
target_link_libraries(tcpsocketstream.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcpsocketstream.gtest COMMAND tcpsocketstream.gtest)
install(TARGETS tcpsocketstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tcpacceptor.gtest tcpacceptor_test.cpp)
target_link_libraries(tcpacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tcpacceptor.gtest COMMAND tcpacceptor.gtest)
install(TARGETS tcpacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlserror.gtest tlserror_test.cpp)
target_link_libraries(tlserror.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tlserror.gtest COMMAND tlserror.gtest)
install(TARGETS tlserror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlssocket.gtest tlssocket_test.cpp)
target_link_libraries(tlssocket.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tlssocket.gtest COMMAND tlssocket.gtest)
install(TARGETS tlssocket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlssocketstream.gtest tlssocketstream_test.cpp)
target_link_libraries(tlssocketstream.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tlssocketstream.gtest COMMAND tlssocketstream.gtest)
install(TARGETS tlssocketstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlsacceptor.gtest tlsacceptor_test.cpp)
target_link_libraries(tlsacceptor.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME tlsacceptor.gtest COMMAND tlsacceptor.gtest)
install(TARGETS tlsacceptor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${JOIN_CORE} GTest::gtest_main)
add_test(NAME openssl.gtest COMMAND openssl.gtest)
install(TARGETS openssl.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
