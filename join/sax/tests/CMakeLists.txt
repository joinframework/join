cmake_minimum_required(VERSION 3.14)

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

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${PROJECT_NAME} gtest gtest_main pthread gcov)
gtest_discover_tests(roundtrip.gtest)
