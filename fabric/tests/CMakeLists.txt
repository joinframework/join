cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(dns_message.gtest dns_message_test.cpp)
target_link_libraries(dns_message.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dns_message.gtest COMMAND dns_message.gtest)
install(TARGETS dns_message.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dns_protocol.gtest dns_protocol_test.cpp)
target_link_libraries(dns_protocol.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dns_protocol.gtest COMMAND dns_protocol.gtest)
install(TARGETS dns_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dns.gtest dns_test.cpp)
target_link_libraries(dns.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dns.gtest COMMAND dns.gtest)
install(TARGETS dns.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mdns_protocol.gtest mdns_protocol_test.cpp)
target_link_libraries(mdns_protocol.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME mdns_protocol.gtest COMMAND mdns_protocol.gtest)
install(TARGETS mdns_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mdns.gtest mdns_test.cpp)
target_link_libraries(mdns.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME mdns.gtest COMMAND mdns.gtest)
install(TARGETS mdns.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dot_protocol.gtest dot_protocol_test.cpp)
target_link_libraries(dot_protocol.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dot_protocol.gtest COMMAND dot_protocol.gtest)
install(TARGETS dot_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dot.gtest dot_test.cpp)
target_link_libraries(dot.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dot.gtest COMMAND dot.gtest)
install(TARGETS dot.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(netlink_protocol.gtest netlink_protocol_test.cpp)
target_link_libraries(netlink_protocol.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME netlink_protocol.gtest COMMAND netlink_protocol.gtest)
install(TARGETS netlink_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(netlink_endpoint.gtest netlink_endpoint_test.cpp)
target_link_libraries(netlink_endpoint.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME netlink_endpoint.gtest COMMAND netlink_endpoint.gtest)
install(TARGETS netlink_endpoint.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(netlink_socket.gtest netlink_socket_test.cpp)
target_link_libraries(netlink_socket.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME netlink_socket.gtest COMMAND netlink_socket.gtest)
install(TARGETS netlink_socket.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(neighbor.gtest neighbor_test.cpp)
target_link_libraries(neighbor.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME neighbor.gtest COMMAND neighbor.gtest)
install(TARGETS neighbor.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(neighbor_manager.gtest neighbor_manager_test.cpp)
target_link_libraries(neighbor_manager.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME neighbor_manager.gtest COMMAND neighbor_manager.gtest)
install(TARGETS neighbor_manager.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(arp.gtest arp_test.cpp)
target_link_libraries(arp.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME arp.gtest COMMAND arp.gtest)
install(TARGETS arp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(ping.gtest ping_test.cpp)
target_link_libraries(ping.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME ping.gtest COMMAND ping.gtest)
install(TARGETS ping.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(interface.gtest interface_test.cpp)
target_link_libraries(interface.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME interface.gtest COMMAND interface.gtest)
install(TARGETS interface.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(interface_manager.gtest interface_manager_test.cpp)
target_link_libraries(interface_manager.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME interface_manager.gtest COMMAND interface_manager.gtest)
install(TARGETS interface_manager.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(route.gtest route_test.cpp)
target_link_libraries(route.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME route.gtest COMMAND route.gtest)
install(TARGETS route.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(route_manager.gtest route_manager_test.cpp)
target_link_libraries(route_manager.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME route_manager.gtest COMMAND route_manager.gtest)
install(TARGETS route_manager.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dhcp_option.gtest dhcp_option_test.cpp)
target_link_libraries(dhcp_option.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dhcp_option.gtest COMMAND dhcp_option.gtest)
install(TARGETS dhcp_option.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dhcp_protocol.gtest dhcp_protocol_test.cpp)
target_link_libraries(dhcp_protocol.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dhcp_protocol.gtest COMMAND dhcp_protocol.gtest)
install(TARGETS dhcp_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dhcp_message.gtest dhcp_message_test.cpp)
target_link_libraries(dhcp_message.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dhcp_message.gtest COMMAND dhcp_message.gtest)
install(TARGETS dhcp_message.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dhcp.gtest dhcp_test.cpp)
target_link_libraries(dhcp.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME dhcp.gtest COMMAND dhcp.gtest)
install(TARGETS dhcp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
