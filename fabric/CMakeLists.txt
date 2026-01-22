cmake_minimum_required(VERSION 3.22.1)

set(PUBLIC_HEADERS
    include/join/resolver.hpp
    include/join/dnsclient.hpp
    include/join/arp.hpp
    include/join/interface.hpp
    include/join/interfacemanager.hpp
)

set(SOURCES
    src/resolver.cpp
    src/arp.cpp
    src/interface.cpp
    src/interfacemanager.cpp
)

add_library(${JOIN_FABRIC} ${SOURCES})
add_library(join::fabric ALIAS ${JOIN_FABRIC})

set_target_properties(${JOIN_FABRIC} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_include_directories(${JOIN_FABRIC}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_FABRIC}
    PUBLIC
        ${JOIN_CORE}
)

install(TARGETS ${JOIN_FABRIC}
    EXPORT joinTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_FABRIC}.pc.in ${JOIN_FABRIC}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_FABRIC}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
