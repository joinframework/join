cmake_minimum_required(VERSION 3.22.1)
project(join)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" JOIN_VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" JOIN_VERSION_MAJOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" JOIN_VERSION_MINOR ${JOIN_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" JOIN_VERSION_PATCH ${JOIN_VERSION})

message(STATUS "${PROJECT_NAME} version: ${JOIN_VERSION}")
configure_file("${PROJECT_SOURCE_DIR}/version.hpp.in" "${PROJECT_SOURCE_DIR}/core/include/join/version.hpp")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to Release as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wformat
    -Wformat-security
    -fstack-protector-strong
    -Wno-address-of-packed-member
)

add_link_options(
    -Wl,--as-needed
    -Wl,-z,relro
    -Wl,-z,now
)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -D_FORTIFY_SOURCE=2 -O3")

option(BUILD_SHARED_LIBS "Build as shared libraries." ON)
option(JOIN_ENABLE_CRYPTO "Enable crypto." ON)
option(JOIN_ENABLE_DATA "Enable data." ON)
option(JOIN_ENABLE_FABRIC "Enable fabric." ON)
option(JOIN_ENABLE_SERVICES "Enable services." ON)
option(JOIN_ENABLE_TESTS "Enable tests." OFF)
option(JOIN_ENABLE_COVERAGE "Enable coverage." OFF)

SET(JOIN_CORE ${PROJECT_NAME}_core)
SET(JOIN_CRYPTO ${PROJECT_NAME}_crypto)
SET(JOIN_DATA ${PROJECT_NAME}_data)
SET(JOIN_FABRIC ${PROJECT_NAME}_fabric)
SET(JOIN_SERVICES ${PROJECT_NAME}_services)

if(JOIN_ENABLE_TESTS)
    enable_testing()
endif()

if(JOIN_ENABLE_COVERAGE)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(FATAL_ERROR "Code coverage results with a non-Debug build may be misleading")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Enabling coverage for GCC")
        add_compile_options(--coverage -fprofile-update=atomic)
        add_link_options(--coverage)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Enabling coverage for Clang")
        add_compile_options(--coverage -fprofile-update=atomic)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "Code coverage requires GCC")
    endif()
endif()

add_subdirectory(core)

if(JOIN_ENABLE_CRYPTO)
    add_subdirectory(crypto)
endif()

if(JOIN_ENABLE_DATA)
    add_subdirectory(data)
endif()

if(JOIN_ENABLE_FABRIC)
    add_subdirectory(fabric)
endif()

if(JOIN_ENABLE_SERVICES)
    add_subdirectory(services)
endif()

include(CMakePackageConfigHelpers)

install(EXPORT joinTargets
    NAMESPACE join::
    DESTINATION lib/cmake/join
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/joinConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/joinConfig.cmake
    INSTALL_DESTINATION lib/cmake/join
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/joinConfigVersion.cmake
    VERSION ${JOIN_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/joinConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/joinConfigVersion.cmake
    DESTINATION lib/cmake/join
)
