cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME base64.gtest COMMAND base64.gtest)
install(TARGETS base64.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(openssl.gtest openssl_test.cpp)
target_link_libraries(openssl.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME openssl.gtest COMMAND openssl.gtest)
install(TARGETS openssl.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(digest_error.gtest digest_error_test.cpp)
target_link_libraries(digest_error.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME digest_error.gtest COMMAND digest_error.gtest)
install(TARGETS digest_error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(digest.gtest digest_test.cpp)
target_link_libraries(digest.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME digest.gtest COMMAND digest.gtest)
install(TARGETS digest.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(hmac.gtest hmac_test.cpp)
target_link_libraries(hmac.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME hmac.gtest COMMAND hmac.gtest)
install(TARGETS hmac.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_key.gtest tls_key_test.cpp)
target_link_libraries(tls_key.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_key.gtest COMMAND tls_key.gtest)
install(TARGETS tls_key.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(signature.gtest signature_test.cpp)
target_link_libraries(signature.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME signature.gtest COMMAND signature.gtest)
install(TARGETS signature.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_error.gtest tls_error_test.cpp)
target_link_libraries(tls_error.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_error.gtest COMMAND tls_error.gtest)
install(TARGETS tls_error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_context.gtest tls_context_test.cpp)
target_link_libraries(tls_context.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_context.gtest COMMAND tls_context.gtest)
install(TARGETS tls_context.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_protocol.gtest tls_protocol_test.cpp)
target_link_libraries(tls_protocol.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_protocol.gtest COMMAND tls_protocol.gtest)
install(TARGETS tls_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_wrapper.gtest tls_wrapper_test.cpp)
target_link_libraries(tls_wrapper.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_wrapper.gtest COMMAND tls_wrapper.gtest)
install(TARGETS tls_wrapper.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tls_stream.gtest tls_stream_test.cpp)
target_link_libraries(tls_stream.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tls_stream.gtest COMMAND tls_stream.gtest)
install(TARGETS tls_stream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(dtls_wrapper.gtest dtls_wrapper_test.cpp)
target_link_libraries(dtls_wrapper.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME dtls_wrapper.gtest COMMAND dtls_wrapper.gtest)
install(TARGETS dtls_wrapper.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
