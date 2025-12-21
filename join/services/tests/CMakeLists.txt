cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(chunkstream.gtest chunkstream_test.cpp)
target_link_libraries(chunkstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(chunkstream.gtest)

add_executable(httperror.gtest httperror_test.cpp)
target_link_libraries(httperror.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httperror.gtest)

add_executable(httprequest.gtest httprequest_test.cpp)
target_link_libraries(httprequest.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httprequest.gtest)

add_executable(httpresponse.gtest httpresponse_test.cpp)
target_link_libraries(httpresponse.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(httpresponse.gtest)

add_executable(http.gtest http_test.cpp)
target_link_libraries(http.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(http.gtest)

add_executable(https.gtest https_test.cpp)
target_link_libraries(https.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(https.gtest)

add_executable(mailsender.gtest mailsender_test.cpp)
target_link_libraries(mailsender.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(mailsender.gtest)

add_executable(mailrecipient.gtest mailrecipient_test.cpp)
target_link_libraries(mailrecipient.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(mailrecipient.gtest)

add_executable(mailmessage.gtest mailmessage_test.cpp)
target_link_libraries(mailmessage.gtest ${PROJECT_NAME} GTest::gtest_main GTest::gmock_main pthread)
gtest_discover_tests(mailmessage.gtest)

add_executable(smtp.gtest smtp_test.cpp)
target_link_libraries(smtp.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(smtp.gtest)

add_executable(smtps.gtest smtps_test.cpp)
target_link_libraries(smtps.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(smtps.gtest)
