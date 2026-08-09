cmake_minimum_required(VERSION 3.22.1)

find_package(OpenSSL REQUIRED)

set(PUBLIC_HEADERS
    include/join/base64.hpp
    include/join/openssl.hpp
    include/join/digest.hpp
    include/join/hmac.hpp
    include/join/tls_key.hpp
    include/join/signature.hpp
    include/join/tls_error.hpp
    include/join/tls_context.hpp
    include/join/tls.hpp
    include/join/tls_wrapper.hpp
    include/join/tls_stream.hpp
    include/join/dtls_wrapper.hpp
    include/join/tls_protocol.hpp
)

set(SOURCES
    src/base64.cpp
    src/openssl.cpp
    src/digest.cpp
    src/hmac.cpp
    src/tls_key.cpp
    src/signature.cpp
    src/tls_error.cpp
    src/tls_context.cpp
)

add_library(${JOIN_CRYPTO} ${SOURCES})
add_library(join::crypto ALIAS ${JOIN_CRYPTO})

set_target_properties(${JOIN_CRYPTO} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_include_directories(${JOIN_CRYPTO}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_CRYPTO}
    PUBLIC
        ${JOIN_CORE}
        OpenSSL::SSL
        OpenSSL::Crypto
)

install(TARGETS ${JOIN_CRYPTO}
    EXPORT joinTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/join
)

configure_file(${JOIN_CRYPTO}.pc.in ${JOIN_CRYPTO}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_CRYPTO}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
