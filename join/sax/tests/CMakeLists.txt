cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

include(GoogleTest)

add_executable(dtoa.gtest dtoa_test.cpp)
target_link_libraries(dtoa.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(dtoa.gtest)

add_executable(value.gtest value_test.cpp)
target_link_libraries(value.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(value.gtest)

add_executable(jsonreader.gtest jsonreader_test.cpp)
target_link_libraries(jsonreader.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(jsonreader.gtest)

