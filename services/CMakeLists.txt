cmake_minimum_required(VERSION 3.22.1)

find_package(ZLIB REQUIRED)

set(PUBLIC_HEADERS
    include/join/cache.hpp
    include/join/chunkstream.hpp
    include/join/httpmessage.hpp
    include/join/httpclient.hpp
    include/join/httpserver.hpp
    include/join/mailmessage.hpp
    include/join/smtpclient.hpp
)

set(SOURCES
    src/cache.cpp
    src/chunkstream.cpp
    src/httpmessage.cpp
    src/mailmessage.cpp
)

add_library(${JOIN_SERVICES} ${SOURCES})
add_library(join::services ALIAS ${JOIN_SERVICES})

set_target_properties(${JOIN_SERVICES} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_include_directories(${JOIN_SERVICES}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_SERVICES}
    PUBLIC
        ${JOIN_FABRIC}
        ${JOIN_DATA}
        ${JOIN_CRYPTO}
        ${JOIN_CORE}
)

install(TARGETS ${JOIN_SERVICES}
    EXPORT joinTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_SERVICES}.pc.in ${JOIN_SERVICES}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_SERVICES}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
