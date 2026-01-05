cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(dtoa.gtest dtoa_test.cpp)
target_link_libraries(dtoa.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(dtoa.gtest)

add_executable(stringview.gtest stringview_test.cpp)
target_link_libraries(stringview.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(stringview.gtest)

add_executable(streamview.gtest streamview_test.cpp)
target_link_libraries(streamview.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(streamview.gtest)

add_executable(value.gtest value_test.cpp)
target_link_libraries(value.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(value.gtest)

add_executable(saxerror.gtest saxerror_test.cpp)
target_link_libraries(saxerror.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(saxerror.gtest)

add_executable(packreader.gtest packreader_test.cpp)
target_link_libraries(packreader.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(packreader.gtest)

add_executable(packwriter.gtest packwriter_test.cpp)
target_link_libraries(packwriter.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(packwriter.gtest)

add_executable(jsonerror.gtest jsonerror_test.cpp)
target_link_libraries(jsonerror.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonerror.gtest)

add_executable(jsonreader.gtest jsonreader_test.cpp)
target_link_libraries(jsonreader.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonreader.gtest)

add_executable(jsonwriter.gtest jsonwriter_test.cpp)
target_link_libraries(jsonwriter.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsonwriter.gtest)

add_executable(jsoncanonicalizer.gtest jsoncanonicalizer_test.cpp)
target_link_libraries(jsoncanonicalizer.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(jsoncanonicalizer.gtest)

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(roundtrip.gtest)

add_executable(parse.gtest parse_test.cpp)
target_link_libraries(parse.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(parse.gtest)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(zstream.gtest zstream_test.cpp)
target_link_libraries(zstream.gtest ${JOIN_DATA} GTest::gtest_main)
gtest_discover_tests(zstream.gtest)
