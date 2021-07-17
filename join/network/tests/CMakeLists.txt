cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(macaddress.gtest macaddress_test.cpp)
target_link_libraries(macaddress.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(macaddress.gtest)

add_executable(ipaddress.gtest ipaddress_test.cpp)
target_link_libraries(ipaddress.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(ipaddress.gtest)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(endpoint.gtest)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(protocol.gtest)

add_executable(unixdgramsocket.gtest unixdgramsocket_test.cpp)
target_link_libraries(unixdgramsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(unixdgramsocket.gtest)

add_executable(unixacceptor.gtest unixacceptor_test.cpp)
target_link_libraries(unixacceptor.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(unixacceptor.gtest)

add_executable(unixstreamsocket.gtest unixstreamsocket_test.cpp)
target_link_libraries(unixstreamsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(unixstreamsocket.gtest)

add_executable(resolver.gtest resolver_test.cpp)
target_link_libraries(resolver.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(resolver.gtest)

add_executable(rawsocket.gtest rawsocket_test.cpp)
target_link_libraries(rawsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(rawsocket.gtest)

add_executable(udpsocket.gtest udpsocket_test.cpp)
target_link_libraries(udpsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(udpsocket.gtest)

add_executable(icmpsocket.gtest icmpsocket_test.cpp)
target_link_libraries(icmpsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(icmpsocket.gtest)

add_executable(tcpacceptor.gtest tcpacceptor_test.cpp)
target_link_libraries(tcpacceptor.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(tcpacceptor.gtest)

add_executable(tcpsocket.gtest tcpsocket_test.cpp)
target_link_libraries(tcpsocket.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(tcpsocket.gtest)
