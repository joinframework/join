cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(base64.gtest base64_test.cpp)
target_link_libraries(base64.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME base64.gtest COMMAND base64.gtest)
install(TARGETS base64.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(digesterror.gtest digesterror_test.cpp)
target_link_libraries(digesterror.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME digesterror.gtest COMMAND digesterror.gtest)
install(TARGETS digesterror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(digest.gtest digest_test.cpp)
target_link_libraries(digest.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME digest.gtest COMMAND digest.gtest)
install(TARGETS digest.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(hmac.gtest hmac_test.cpp)
target_link_libraries(hmac.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME hmac.gtest COMMAND hmac.gtest)
install(TARGETS hmac.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(tlskey.gtest tlskey_test.cpp)
target_link_libraries(tlskey.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME tlskey.gtest COMMAND tlskey.gtest)
install(TARGETS tlskey.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(signature.gtest signature_test.cpp)
target_link_libraries(signature.gtest ${JOIN_CRYPTO} GTest::gtest_main)
add_test(NAME signature.gtest COMMAND signature.gtest)
install(TARGETS signature.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
