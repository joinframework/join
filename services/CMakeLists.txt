cmake_minimum_required(VERSION 3.22.1)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

set(PRIVATE_HEADER_DIRS
    src
)

set(PUBLIC_HEADER_DIRS
    include
)

set(PUBLIC_HEADERS
    include/join/chunkstream.hpp
    include/join/httpmessage.hpp
    include/join/httpclient.hpp
    include/join/httpserver.hpp
    include/join/mailmessage.hpp
    include/join/smtpclient.hpp
)

set(SOURCES
    src/chunkstream.cpp
    src/httpmessage.cpp
    src/mailmessage.cpp
)

add_library(${JOIN_SERVICES} ${SOURCES})
set_target_properties(${JOIN_SERVICES} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${JOIN_SERVICES} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${JOIN_SERVICES} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${JOIN_SERVICES} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${JOIN_SERVICES} ${JOIN_CRYPTO} ${JOIN_DATA} ${JOIN_FABRIC} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB pthread)

install(TARGETS ${JOIN_SERVICES}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_SERVICES}.pc.in ${JOIN_SERVICES}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_SERVICES}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
