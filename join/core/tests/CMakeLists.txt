cmake_minimum_required(VERSION 3.14)

find_package(GTest REQUIRED)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(error.gtest)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(utils.gtest)

add_executable(filesystem.gtest filesystem_test.cpp)
target_link_libraries(filesystem.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(filesystem.gtest)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(traits.gtest)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(variant.gtest)

add_executable(view.gtest view_test.cpp)
target_link_libraries(view.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(view.gtest)

add_executable(cache.gtest cache_test.cpp)
target_link_libraries(cache.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(cache.gtest)

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(reactor.gtest)

add_executable(zstream.gtest zstream_test.cpp)
target_link_libraries(zstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(zstream.gtest)
