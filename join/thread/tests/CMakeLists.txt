cmake_minimum_required(VERSION 3.14)

find_package(GTest REQUIRED)

add_executable(mutex.gtest mutex_test.cpp)
target_link_libraries(mutex.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(mutex.gtest)

add_executable(sharedmutex.gtest sharedmutex_test.cpp)
target_link_libraries(sharedmutex.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(sharedmutex.gtest)

add_executable(condition.gtest condition_test.cpp)
target_link_libraries(condition.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(condition.gtest)

add_executable(sharedcondition.gtest sharedcondition_test.cpp)
target_link_libraries(sharedcondition.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(sharedcondition.gtest)

add_executable(thread.gtest thread_test.cpp)
target_link_libraries(thread.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(thread.gtest)

add_executable(threadpool.gtest threadpool_test.cpp)
target_link_libraries(threadpool.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(threadpool.gtest)
