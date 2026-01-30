cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(dtoa.gtest dtoa_test.cpp)
target_link_libraries(dtoa.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(dtoa.gtest)
install(TARGETS dtoa.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(stringview.gtest stringview_test.cpp)
target_link_libraries(stringview.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(stringview.gtest)
install(TARGETS stringview.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(streamview.gtest streamview_test.cpp)
target_link_libraries(streamview.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(streamview.gtest)
install(TARGETS streamview.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(value.gtest value_test.cpp)
target_link_libraries(value.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(value.gtest)
install(TARGETS value.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(saxerror.gtest saxerror_test.cpp)
target_link_libraries(saxerror.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(saxerror.gtest)
install(TARGETS saxerror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(packreader.gtest packreader_test.cpp)
target_link_libraries(packreader.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(packreader.gtest)
install(TARGETS packreader.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(packwriter.gtest packwriter_test.cpp)
target_link_libraries(packwriter.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(packwriter.gtest)
install(TARGETS packwriter.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(jsonerror.gtest jsonerror_test.cpp)
target_link_libraries(jsonerror.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonerror.gtest)
install(TARGETS jsonerror.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(jsonreader.gtest jsonreader_test.cpp)
target_link_libraries(jsonreader.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonreader.gtest)
install(TARGETS jsonreader.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(jsonwriter.gtest jsonwriter_test.cpp)
target_link_libraries(jsonwriter.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonwriter.gtest)
install(TARGETS jsonwriter.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(jsoncanonicalizer.gtest jsoncanonicalizer_test.cpp)
target_link_libraries(jsoncanonicalizer.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsoncanonicalizer.gtest)
install(TARGETS jsoncanonicalizer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(roundtrip.gtest)
install(TARGETS roundtrip.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(parse.gtest parse_test.cpp)
target_link_libraries(parse.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(parse.gtest)
install(TARGETS parse.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(zstream.gtest zstream_test.cpp)
target_link_libraries(zstream.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(zstream.gtest)
install(TARGETS zstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
