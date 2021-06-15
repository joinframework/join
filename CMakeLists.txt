cmake_minimum_required(VERSION 3.14)
project(join)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-address-of-packed-member")

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" JOIN_VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" JOIN_VERSION_MAJOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" JOIN_VERSION_MINOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" JOIN_VERSION_PATCH ${JOIN_VERSION})

message(STATUS "join version: ${JOIN_VERSION}")

option(BUILD_SHARED_LIBS "Build as shared libraries." ON)
option(JOIN_ENABLE_TESTS "Enable tests." OFF)

if(JOIN_ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(googletest)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    enable_testing()
endif()

add_subdirectory(join)
