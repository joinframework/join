cmake_minimum_required(VERSION 3.22.1)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

set(PRIVATE_HEADER_DIRS
    core/src
    crypto/src
    data/src
    fabric/src
    services/src
)

set(PUBLIC_HEADER_DIRS
    core/include
    crypto/include
    data/include
    fabric/include
    services/include
)

set(PUBLIC_HEADERS
    core/include/join/version.hpp
    core/include/join/error.hpp
    core/include/join/utils.hpp
    core/include/join/filesystem.hpp
    core/include/join/traits.hpp
    core/include/join/variant.hpp
    core/include/join/view.hpp
    core/include/join/cache.hpp
    core/include/join/reactor.hpp
    core/include/join/streambuf.hpp
    core/include/join/shared.hpp
    core/include/join/mutex.hpp
    core/include/join/condition.hpp
    core/include/join/semaphore.hpp
    core/include/join/thread.hpp
    core/include/join/threadpool.hpp
    core/include/join/macaddress.hpp
    core/include/join/ipaddress.hpp
    core/include/join/endpoint.hpp
    core/include/join/protocol.hpp
    core/include/join/socket.hpp
    core/include/join/socketstream.hpp
    core/include/join/acceptor.hpp
    core/include/join/openssl.hpp
    crypto/include/join/base64.hpp
    crypto/include/join/digest.hpp
    crypto/include/join/hmac.hpp
    crypto/include/join/tlskey.hpp
    crypto/include/join/signature.hpp
    data/include/join/diyfp.hpp
    data/include/join/atodpow.hpp
    data/include/join/dtoapow.hpp
    data/include/join/dtoa.hpp
    data/include/join/value.hpp
    data/include/join/sax.hpp
    data/include/join/pack.hpp
    data/include/join/json.hpp
    data/include/join/zstream.hpp
    fabric/include/join/resolver.hpp
    fabric/include/join/arp.hpp
    fabric/include/join/interface.hpp
    fabric/include/join/interfacemanager.hpp
    services/include/join/chunkstream.hpp
    services/include/join/httpmessage.hpp
    services/include/join/httpclient.hpp
    services/include/join/httpserver.hpp
    services/include/join/mailmessage.hpp
    services/include/join/smtpclient.hpp
)

set(SOURCES
    core/src/error.cpp
    core/src/cache.cpp
    core/src/reactor.cpp
    core/src/mutex.cpp
    core/src/condition.cpp
    core/src/semaphore.cpp
    core/src/thread.cpp
    core/src/threadpool.cpp
    core/src/openssl.cpp
    core/src/macaddress.cpp
    core/src/ipaddress.cpp
    core/src/socket.cpp
    crypto/src/base64.cpp
    crypto/src/digest.cpp
    crypto/src/hmac.cpp
    crypto/src/tlskey.cpp
    crypto/src/signature.cpp
    data/src/value.cpp
    data/src/sax.cpp
    data/src/json.cpp
    data/src/zstream.cpp
    fabric/src/resolver.cpp
    fabric/src/arp.cpp
    fabric/src/interface.cpp
    fabric/src/interfacemanager.cpp
    services/src/chunkstream.cpp
    services/src/httpmessage.cpp
    services/src/mailmessage.cpp
)

add_library(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB pthread)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(data)
add_subdirectory(fabric)
add_subdirectory(services)
