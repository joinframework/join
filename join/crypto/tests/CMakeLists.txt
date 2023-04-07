cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(openssl.gtest)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(base64.gtest)

add_executable(sig.gtest sig_test.cpp)
target_link_libraries(sig.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(sig.gtest)

add_executable(sigerror.gtest sigerror_test.cpp)
target_link_libraries(sigerror.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(sigerror.gtest)
