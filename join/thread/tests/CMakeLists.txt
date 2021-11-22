cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(thread.gtest)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(mutex.gtest)
