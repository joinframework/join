cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(openssl.gtest)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(base64.gtest)

add_executable(digesterror.gtest digesterror_test.cpp)
target_link_libraries(digesterror.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(digesterror.gtest)

add_executable(digest.gtest digest_test.cpp)
target_link_libraries(digest.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(digest.gtest)

add_executable(hmac.gtest hmac_test.cpp)
target_link_libraries(hmac.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(hmac.gtest)

add_executable(tlskey.gtest tlskey_test.cpp)
target_link_libraries(tlskey.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(tlskey.gtest)

add_executable(signature.gtest signature_test.cpp)
target_link_libraries(signature.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(signature.gtest)
