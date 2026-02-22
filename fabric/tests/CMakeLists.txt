cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(arp.gtest arp_test.cpp)
target_link_libraries(arp.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME arp.gtest COMMAND arp.gtest)
install(TARGETS arp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(resolver.gtest resolver_test.cpp)
target_link_libraries(resolver.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME resolver.gtest COMMAND resolver.gtest)
install(TARGETS resolver.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(interface.gtest interface_test.cpp)
target_link_libraries(interface.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME interface.gtest COMMAND interface.gtest)
install(TARGETS interface.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(interfacemanager.gtest interfacemanager_test.cpp)
target_link_libraries(interfacemanager.gtest ${JOIN_FABRIC} GTest::gtest_main)
add_test(NAME interfacemanager.gtest COMMAND interfacemanager.gtest)
install(TARGETS interfacemanager.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
