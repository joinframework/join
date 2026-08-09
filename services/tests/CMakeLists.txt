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

add_executable(http_protocol.gtest http_protocol_test.cpp)
target_link_libraries(http_protocol.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http_protocol.gtest COMMAND http_protocol.gtest)
install(TARGETS http_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

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

add_executable(http.gtest http_test.cpp)
target_link_libraries(http.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http.gtest COMMAND http.gtest)
install(TARGETS http.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(https.gtest https_test.cpp)
target_link_libraries(https.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME https.gtest COMMAND https.gtest)
install(TARGETS https.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp_sender.gtest smtp_sender_test.cpp)
target_link_libraries(smtp_sender.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtp_sender.gtest COMMAND smtp_sender.gtest)
install(TARGETS smtp_sender.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp_protocol.gtest smtp_protocol_test.cpp)
target_link_libraries(smtp_protocol.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtp_protocol.gtest COMMAND smtp_protocol.gtest)
install(TARGETS smtp_protocol.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp_recipient.gtest smtp_recipient_test.cpp)
target_link_libraries(smtp_recipient.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtp_recipient.gtest COMMAND smtp_recipient.gtest)
install(TARGETS smtp_recipient.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp_message.gtest smtp_message_test.cpp)
target_link_libraries(smtp_message.gtest ${JOIN_SERVICES} GTest::gtest_main GTest::gmock_main)
add_test(NAME smtp_message.gtest COMMAND smtp_message.gtest)
install(TARGETS smtp_message.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp.gtest smtp_test.cpp)
target_link_libraries(smtp.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtp.gtest COMMAND smtp.gtest)
install(TARGETS smtp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtps.gtest smtps_test.cpp)
target_link_libraries(smtps.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtps.gtest COMMAND smtps.gtest)
install(TARGETS smtps.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
