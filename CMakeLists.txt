cmake_minimum_required(VERSION 3.14)
project(join)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" JOIN_VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" JOIN_VERSION_MAJOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" JOIN_VERSION_MINOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" JOIN_VERSION_PATCH ${JOIN_VERSION})

message(STATUS "${PROJECT_NAME} version: ${JOIN_VERSION}")
configure_file("${PROJECT_SOURCE_DIR}/version.hpp.in" "${PROJECT_SOURCE_DIR}/join/core/include/join/version.hpp")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to Release as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -Wformat -Wformat-security -Wall -Wextra -Werror -Wno-address-of-packed-member" CACHE STRING "CXXFLAGS" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0 -g" CACHE STRING "CXXFLAGS for debug" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O2 -D_FORTIFY_SOURCE=2" CACHE STRING "CXXFLAGS for release" FORCE)
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--as-needed -Wl,-z,relro,-z,now" CACHE STRING "LDFLAGS")

option(BUILD_SHARED_LIBS "Build as shared libraries." ON)
option(JOIN_ENABLE_TESTS "Enable tests." OFF)
option(JOIN_ENABLE_COVERAGE "Enable coverage." OFF)

if(JOIN_ENABLE_TESTS)
    enable_testing()
endif()

if(JOIN_ENABLE_COVERAGE)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(FATAL_ERROR "Code coverage results with a non-Debug build may be misleading")
    endif()
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage" CACHE STRING "CXXFLAGS" FORCE)
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} --coverage" CACHE STRING "LDFLAGS")
    else()
        message(FATAL_ERROR "Code coverage requires GCC")
    endif()
endif()

add_subdirectory(join)
