cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(base64.gtest)

