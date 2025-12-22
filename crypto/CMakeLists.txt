cmake_minimum_required(VERSION 3.22.1)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

set(PRIVATE_HEADER_DIRS
    src
)

set(PUBLIC_HEADER_DIRS
    include
)

set(PUBLIC_HEADERS
    include/join/base64.hpp
    include/join/digest.hpp
    include/join/hmac.hpp
    include/join/tlskey.hpp
    include/join/signature.hpp
)

set(SOURCES
    src/base64.cpp
    src/digest.cpp
    src/hmac.cpp
    src/tlskey.cpp
    src/signature.cpp
)

add_library(${JOIN_CRYPTO} ${SOURCES})
add_library(join::crypto ALIAS ${JOIN_CRYPTO})

set_target_properties(${JOIN_CRYPTO} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
)

target_include_directories(${JOIN_CRYPTO}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_CRYPTO}
    ${JOIN_CORE}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

install(TARGETS ${JOIN_CRYPTO}
    EXPORT joinTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

configure_file(${JOIN_CRYPTO}.pc.in ${JOIN_CRYPTO}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_CRYPTO}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
