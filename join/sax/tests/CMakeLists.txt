cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

add_executable(dtoa.gtest dtoa_test.cpp)
target_link_libraries(dtoa.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(dtoa.gtest)

add_executable(value.gtest value_test.cpp)
target_link_libraries(value.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(value.gtest)

add_executable(saxerror.gtest saxerror_test.cpp)
target_link_libraries(saxerror.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(saxerror.gtest)

add_executable(packreader.gtest packreader_test.cpp)
target_link_libraries(packreader.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(packreader.gtest)

add_executable(packwriter.gtest packwriter_test.cpp)
target_link_libraries(packwriter.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(packwriter.gtest)

add_executable(jsonerror.gtest jsonerror_test.cpp)
target_link_libraries(jsonerror.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(jsonerror.gtest)

add_executable(jsonreader.gtest jsonreader_test.cpp)
target_link_libraries(jsonreader.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(jsonreader.gtest)

add_executable(jsonwriter.gtest jsonwriter_test.cpp)
target_link_libraries(jsonwriter.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(jsonwriter.gtest)

add_executable(jsoncanonicalizer.gtest jsoncanonicalizer_test.cpp)
target_link_libraries(jsoncanonicalizer.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(jsoncanonicalizer.gtest)

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${PROJECT_NAME} gtest gtest_main pthread)
gtest_discover_tests(roundtrip.gtest)
