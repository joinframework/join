cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(cache.gtest cache_test.cpp)
target_link_libraries(cache.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME cache.gtest COMMAND cache.gtest)
install(TARGETS cache.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(chunk_stream.gtest chunk_stream_test.cpp)
target_link_libraries(chunk_stream.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME chunk_stream.gtest COMMAND chunk_stream.gtest)
install(TARGETS chunk_stream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(http_error.gtest http_error_test.cpp)
target_link_libraries(http_error.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http_error.gtest COMMAND http_error.gtest)
install(TARGETS http_error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(http_request.gtest http_request_test.cpp)
target_link_libraries(http_request.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http_request.gtest COMMAND http_request.gtest)
install(TARGETS http_request.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(http_response.gtest http_response_test.cpp)
target_link_libraries(http_response.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http_response.gtest COMMAND http_response.gtest)
install(TARGETS http_response.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

# add_executable(http.gtest http_test.cpp)
# target_link_libraries(http.gtest ${JOIN_SERVICES} GTest::gtest_main)
# add_test(NAME http.gtest COMMAND http.gtest)
# install(TARGETS http.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

# add_executable(https.gtest https_test.cpp)
# target_link_libraries(https.gtest ${JOIN_SERVICES} GTest::gtest_main)
# add_test(NAME https.gtest COMMAND https.gtest)
# install(TARGETS https.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mail_sender.gtest mail_sender_test.cpp)
target_link_libraries(mail_sender.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME mail_sender.gtest COMMAND mail_sender.gtest)
install(TARGETS mail_sender.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mail_recipient.gtest mail_recipient_test.cpp)
target_link_libraries(mail_recipient.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME mail_recipient.gtest COMMAND mail_recipient.gtest)
install(TARGETS mail_recipient.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mail_message.gtest mail_message_test.cpp)
target_link_libraries(mail_message.gtest ${JOIN_SERVICES} GTest::gtest_main GTest::gmock_main)
add_test(NAME mail_message.gtest COMMAND mail_message.gtest)
install(TARGETS mail_message.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

# add_executable(smtp.gtest smtp_test.cpp)
# target_link_libraries(smtp.gtest ${JOIN_SERVICES} GTest::gtest_main)
# add_test(NAME smtp.gtest COMMAND smtp.gtest)
# install(TARGETS smtp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

# add_executable(smtps.gtest smtps_test.cpp)
# target_link_libraries(smtps.gtest ${JOIN_SERVICES} GTest::gtest_main)
# add_test(NAME smtps.gtest COMMAND smtps.gtest)
# install(TARGETS smtps.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
