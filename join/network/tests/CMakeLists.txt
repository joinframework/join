cmake_minimum_required(VERSION 3.14)

find_package(GTest REQUIRED)

add_executable(macaddress.gtest macaddress_test.cpp)
target_link_libraries(macaddress.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(macaddress.gtest)

add_executable(ipaddress.gtest ipaddress_test.cpp)
target_link_libraries(ipaddress.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(ipaddress.gtest)

add_executable(endpoint.gtest endpoint_test.cpp)
target_link_libraries(endpoint.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(endpoint.gtest)

add_executable(protocol.gtest protocol_test.cpp)
target_link_libraries(protocol.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(protocol.gtest)

add_executable(unixdgramsocket.gtest unixdgramsocket_test.cpp)
target_link_libraries(unixdgramsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(unixdgramsocket.gtest)

add_executable(unixstreamsocket.gtest unixstreamsocket_test.cpp)
target_link_libraries(unixstreamsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(unixstreamsocket.gtest)

add_executable(unixstreamacceptor.gtest unixstreamacceptor_test.cpp)
target_link_libraries(unixstreamacceptor.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(unixstreamacceptor.gtest)

add_executable(resolver.gtest resolver_test.cpp)
target_link_libraries(resolver.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(resolver.gtest)

add_executable(rawsocket.gtest rawsocket_test.cpp)
target_link_libraries(rawsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(rawsocket.gtest)

add_executable(udpsocket.gtest udpsocket_test.cpp)
target_link_libraries(udpsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(udpsocket.gtest)

add_executable(icmpsocket.gtest icmpsocket_test.cpp)
target_link_libraries(icmpsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(icmpsocket.gtest)

add_executable(tcpsocket.gtest tcpsocket_test.cpp)
target_link_libraries(tcpsocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tcpsocket.gtest)

add_executable(tcpsocketstream.gtest tcpsocketstream_test.cpp)
target_link_libraries(tcpsocketstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tcpsocketstream.gtest)

add_executable(tcpacceptor.gtest tcpacceptor_test.cpp)
target_link_libraries(tcpacceptor.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tcpacceptor.gtest)

add_executable(tlserror.gtest tlserror_test.cpp)
target_link_libraries(tlserror.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tlserror.gtest)

add_executable(tlssocket.gtest tlssocket_test.cpp)
target_link_libraries(tlssocket.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tlssocket.gtest)

add_executable(tlssocketstream.gtest tlssocketstream_test.cpp)
target_link_libraries(tlssocketstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tlssocketstream.gtest)

add_executable(tlsacceptor.gtest tlsacceptor_test.cpp)
target_link_libraries(tlsacceptor.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tlsacceptor.gtest)

add_executable(arp.gtest arp_test.cpp)
target_link_libraries(arp.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(arp.gtest)

add_executable(chunkstream.gtest chunkstream_test.cpp)
target_link_libraries(chunkstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(chunkstream.gtest)

add_executable(httperror.gtest httperror_test.cpp)
target_link_libraries(httperror.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httperror.gtest)

add_executable(httprequest.gtest httprequest_test.cpp)
target_link_libraries(httprequest.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httprequest.gtest)

add_executable(httpresponse.gtest httpresponse_test.cpp)
target_link_libraries(httpresponse.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httpresponse.gtest)

add_executable(http.gtest http_test.cpp)
target_link_libraries(http.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(http.gtest)

add_executable(https.gtest https_test.cpp)
target_link_libraries(https.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(https.gtest)

add_executable(mailsender.gtest mailsender_test.cpp)
target_link_libraries(mailsender.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(mailsender.gtest)

add_executable(mailrecipient.gtest mailrecipient_test.cpp)
target_link_libraries(mailrecipient.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(mailrecipient.gtest)

add_executable(mailmessage.gtest mailmessage_test.cpp)
target_link_libraries(mailmessage.gtest ${PROJECT_NAME} GTest::gtest_main GTest::gmock_main pthread)
gtest_discover_tests(mailmessage.gtest)

add_executable(smtp.gtest smtp_test.cpp)
target_link_libraries(smtp.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(smtp.gtest)

add_executable(smtps.gtest smtps_test.cpp)
target_link_libraries(smtps.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(smtps.gtest)

add_executable(interface.gtest interface_test.cpp)
target_link_libraries(interface.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(interface.gtest)

add_executable(interfacemanager.gtest interfacemanager_test.cpp)
target_link_libraries(interfacemanager.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(interfacemanager.gtest)
