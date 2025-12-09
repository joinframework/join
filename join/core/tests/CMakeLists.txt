cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(error.gtest error_test.cpp)
target_link_libraries(error.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(error.gtest)

add_executable(utils.gtest utils_test.cpp)
target_link_libraries(utils.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(utils.gtest)

add_executable(filesystem.gtest filesystem_test.cpp)
target_link_libraries(filesystem.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(filesystem.gtest)

add_executable(traits.gtest traits_test.cpp)
target_link_libraries(traits.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(traits.gtest)

add_executable(variant.gtest variant_test.cpp)
target_link_libraries(variant.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(variant.gtest)

add_executable(view.gtest view_test.cpp)
target_link_libraries(view.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(view.gtest)

add_executable(cache.gtest cache_test.cpp)
target_link_libraries(cache.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(cache.gtest)

add_executable(reactor.gtest reactor_test.cpp)
target_link_libraries(reactor.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(reactor.gtest)

add_executable(zstream.gtest zstream_test.cpp)
target_link_libraries(zstream.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(zstream.gtest)

add_executable(realtimetimer.gtest realtimetimer_test.cpp)
target_link_libraries(realtimetimer.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(realtimetimer.gtest)

add_executable(monotonictimer.gtest monotonictimer_test.cpp)
target_link_libraries(monotonictimer.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(monotonictimer.gtest)

add_executable(sharedmem.gtest sharedmem_test.cpp)
target_link_libraries(sharedmem.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(sharedmem.gtest)

add_executable(spsc.gtest spsc_test.cpp)
target_link_libraries(spsc.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(spsc.gtest)

add_executable(mpsc.gtest mpsc_test.cpp)
target_link_libraries(mpsc.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(mpsc.gtest)

add_executable(mpmc.gtest mpmc_test.cpp)
target_link_libraries(mpmc.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(mpmc.gtest)

add_executable(spscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(spscendpoint.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(spscendpoint.gtest)

add_executable(mpscendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpscendpoint.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(mpscendpoint.gtest)

add_executable(mpmcendpoint.gtest spscendpoint_test.cpp)
target_link_libraries(mpmcendpoint.gtest ${PROJECT_NAME} GTest::gtest_main pthread rt)
gtest_discover_tests(mpmcendpoint.gtest)
