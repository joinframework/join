cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(error.gtest)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(utils.gtest)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(traits.gtest)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(variant.gtest)

add_executable(view.gtest view_test.cpp)
target_link_libraries(view.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(view.gtest)

