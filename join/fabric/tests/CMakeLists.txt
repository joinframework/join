cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(arp.gtest arp_test.cpp)
target_link_libraries(arp.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(arp.gtest)

add_executable(resolver.gtest resolver_test.cpp)
target_link_libraries(resolver.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(resolver.gtest)

add_executable(interface.gtest interface_test.cpp)
target_link_libraries(interface.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(interface.gtest)

add_executable(interfacemanager.gtest interfacemanager_test.cpp)
target_link_libraries(interfacemanager.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(interfacemanager.gtest)
