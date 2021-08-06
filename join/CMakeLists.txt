cmake_minimum_required(VERSION 3.14)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(PRIVATE_HEADER_DIRS
    core/src
    crypto/src
    network/src
    sax/src
)

set(PUBLIC_HEADER_DIRS
    core/include
    crypto/include
    network/include
    sax/include
)

set(PUBLIC_HEADERS
    core/include/join/error.hpp
    core/include/join/traits.hpp
    core/include/join/variant.hpp
    core/include/join/view.hpp
    crypto/include/join/base64.hpp
    crypto/include/join/openssl.hpp
    network/include/join/macaddress.hpp
    network/include/join/ipaddress.hpp
    network/include/join/endpoint.hpp
    network/include/join/protocol.hpp
    network/include/join/resolver.hpp
    network/include/join/observer.hpp
    network/include/join/socket.hpp
    network/include/join/acceptor.hpp
    sax/include/join/diyfp.hpp
    sax/include/join/dtoa.hpp
    sax/include/join/value.hpp
    sax/include/join/json.hpp
    sax/include/join/sax.hpp
)

set(SOURCES
    core/src/error.cpp
    crypto/src/base64.cpp
    crypto/src/openssl.cpp
    network/src/macaddress.cpp
    network/src/ipaddress.cpp
    sax/src/sax.cpp
)

add_library(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} pthread)

install(TARGETS ${PROJECT_NAME} 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(network)
add_subdirectory(sax)

