cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(utf8.gtest utf8_test.cpp)
target_link_libraries(utf8.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME utf8.gtest COMMAND utf8.gtest)
install(TARGETS utf8.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(utf16.gtest utf16_test.cpp)
target_link_libraries(utf16.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME utf16.gtest COMMAND utf16.gtest)
install(TARGETS utf16.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(value.gtest value_test.cpp)
target_link_libraries(value.gtest ${JOIN_DATA} GTest::gtest_main) 
add_test(NAME value.gtest COMMAND value.gtest)
install(TARGETS value.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(value_builder.gtest value_builder_test.cpp)
target_link_libraries(value_builder.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME value_builder.gtest COMMAND value_builder.gtest)
install(TARGETS value_builder.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sax_error.gtest sax_error_test.cpp)
target_link_libraries(sax_error.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME sax_error.gtest COMMAND sax_error.gtest)
install(TARGETS sax_error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(sax_handler.gtest sax_handler_test.cpp)
target_link_libraries(sax_handler.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME sax_handler.gtest COMMAND sax_handler.gtest)
install(TARGETS sax_handler.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(pack_reader.gtest pack_reader_test.cpp)
target_link_libraries(pack_reader.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME pack_reader.gtest COMMAND pack_reader.gtest)
install(TARGETS pack_reader.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(pack_writer.gtest pack_writer_test.cpp)
target_link_libraries(pack_writer.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME pack_writer.gtest COMMAND pack_writer.gtest)
install(TARGETS pack_writer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(json_error.gtest json_error_test.cpp)
target_link_libraries(json_error.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME json_error.gtest COMMAND json_error.gtest)
install(TARGETS json_error.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(json_reader.gtest json_reader_test.cpp)
target_link_libraries(json_reader.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME json_reader.gtest COMMAND json_reader.gtest)
install(TARGETS json_reader.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(json_writer.gtest json_writer_test.cpp)
target_link_libraries(json_writer.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME json_writer.gtest COMMAND json_writer.gtest)
install(TARGETS json_writer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(json_canonicalizer.gtest json_canonicalizer_test.cpp)
target_link_libraries(json_canonicalizer.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME json_canonicalizer.gtest COMMAND json_canonicalizer.gtest)
install(TARGETS json_canonicalizer.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME roundtrip.gtest COMMAND roundtrip.gtest)
install(TARGETS roundtrip.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(parse.gtest parse_test.cpp)
target_link_libraries(parse.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME parse.gtest COMMAND parse.gtest)
install(TARGETS parse.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)

add_executable(zstream.gtest zstream_test.cpp)
target_link_libraries(zstream.gtest ${JOIN_DATA} GTest::gtest_main)
add_test(NAME zstream.gtest COMMAND zstream.gtest)
install(TARGETS zstream.gtest RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/test)
