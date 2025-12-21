cmake_minimum_required(VERSION 3.22.1)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(base64.gtest)

add_executable(digesterror.gtest digesterror_test.cpp)
target_link_libraries(digesterror.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(digesterror.gtest)

add_executable(digest.gtest digest_test.cpp)
target_link_libraries(digest.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(digest.gtest)

add_executable(hmac.gtest hmac_test.cpp)
target_link_libraries(hmac.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(hmac.gtest)

add_executable(tlskey.gtest tlskey_test.cpp)
target_link_libraries(tlskey.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(tlskey.gtest)

add_executable(signature.gtest signature_test.cpp)
target_link_libraries(signature.gtest ${JOIN_CRYPTO} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto GTest::gtest_main Threads::Threads)
gtest_discover_tests(signature.gtest)
