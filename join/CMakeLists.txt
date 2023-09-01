cmake_minimum_required(VERSION 3.14)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

set(PRIVATE_HEADER_DIRS
    core/src
    thread/src
    crypto/src
    network/src
    sax/src
)

set(PUBLIC_HEADER_DIRS
    core/include
    thread/include
    crypto/include
    network/include
    sax/include
)

set(PUBLIC_HEADERS
    core/include/join/version.hpp
    core/include/join/error.hpp
    core/include/join/utils.hpp
    core/include/join/traits.hpp
    core/include/join/variant.hpp
    core/include/join/view.hpp
    core/include/join/cache.hpp
    core/include/join/reactor.hpp
    core/include/join/streambuf.hpp
    core/include/join/zstream.hpp
    thread/include/join/mutex.hpp
    thread/include/join/condition.hpp
    thread/include/join/thread.hpp
    thread/include/join/threadpool.hpp
    crypto/include/join/openssl.hpp
    crypto/include/join/base64.hpp
    crypto/include/join/digest.hpp
    crypto/include/join/tlskey.hpp
    crypto/include/join/signature.hpp
    network/include/join/macaddress.hpp
    network/include/join/ipaddress.hpp
    network/include/join/endpoint.hpp
    network/include/join/protocol.hpp
    network/include/join/resolver.hpp
    network/include/join/socket.hpp
    network/include/join/socketstream.hpp
    network/include/join/acceptor.hpp
    network/include/join/arp.hpp
    network/include/join/chunkstream.hpp
    network/include/join/httpmessage.hpp
    network/include/join/httpclient.hpp
    sax/include/join/diyfp.hpp
    sax/include/join/dtoa.hpp
    sax/include/join/value.hpp
    sax/include/join/sax.hpp
    sax/include/join/pack.hpp
    sax/include/join/json.hpp
)

set(SOURCES
    core/src/error.cpp
    core/src/cache.cpp
    core/src/reactor.cpp
    core/src/zstream.cpp
    thread/src/mutex.cpp
    thread/src/condition.cpp
    thread/src/thread.cpp
    thread/src/threadpool.cpp
    crypto/src/openssl.cpp
    crypto/src/base64.cpp
    crypto/src/digest.cpp
    crypto/src/tlskey.cpp
    crypto/src/signature.cpp
    network/src/macaddress.cpp
    network/src/ipaddress.cpp
    network/src/resolver.cpp
    network/src/socket.cpp
    network/src/arp.cpp
    network/src/chunkstream.cpp
    network/src/httpmessage.cpp
    sax/src/value.cpp
    sax/src/sax.cpp
    sax/src/json.cpp
)

add_library(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} pthread)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

add_subdirectory(core)
add_subdirectory(thread)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(sax)
