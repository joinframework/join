cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(cache.gtest cache_test.cpp)
target_link_libraries(cache.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME cache.gtest COMMAND cache.gtest)
install(TARGETS cache.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(chunkstream.gtest chunkstream_test.cpp)
target_link_libraries(chunkstream.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME chunkstream.gtest COMMAND chunkstream.gtest)
install(TARGETS chunkstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(httperror.gtest httperror_test.cpp)
target_link_libraries(httperror.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME httperror.gtest COMMAND httperror.gtest)
install(TARGETS httperror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(httprequest.gtest httprequest_test.cpp)
target_link_libraries(httprequest.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME httprequest.gtest COMMAND httprequest.gtest)
install(TARGETS httprequest.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(httpresponse.gtest httpresponse_test.cpp)
target_link_libraries(httpresponse.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME httpresponse.gtest COMMAND httpresponse.gtest)
install(TARGETS httpresponse.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(http.gtest http_test.cpp)
target_link_libraries(http.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME http.gtest COMMAND http.gtest)
install(TARGETS http.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(https.gtest https_test.cpp)
target_link_libraries(https.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME https.gtest COMMAND https.gtest)
install(TARGETS https.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mailsender.gtest mailsender_test.cpp)
target_link_libraries(mailsender.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME mailsender.gtest COMMAND mailsender.gtest)
install(TARGETS mailsender.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mailrecipient.gtest mailrecipient_test.cpp)
target_link_libraries(mailrecipient.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME mailrecipient.gtest COMMAND mailrecipient.gtest)
install(TARGETS mailrecipient.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(mailmessage.gtest mailmessage_test.cpp)
target_link_libraries(mailmessage.gtest ${JOIN_SERVICES} GTest::gtest_main GTest::gmock_main)
add_test(NAME mailmessage.gtest COMMAND mailmessage.gtest)
install(TARGETS mailmessage.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtp.gtest smtp_test.cpp)
target_link_libraries(smtp.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtp.gtest COMMAND smtp.gtest)
install(TARGETS smtp.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(smtps.gtest smtps_test.cpp)
target_link_libraries(smtps.gtest ${JOIN_SERVICES} GTest::gtest_main)
add_test(NAME smtps.gtest COMMAND smtps.gtest)
install(TARGETS smtps.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
