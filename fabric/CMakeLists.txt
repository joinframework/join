cmake_minimum_required(VERSION 3.22.1)

set(PUBLIC_HEADERS
    include/join/dns_protocol.hpp
    include/join/mdns_protocol.hpp
    include/join/dot_protocol.hpp
    include/join/dns_message.hpp
    include/join/resolver.hpp
    include/join/nameserver.hpp
    include/join/netlink_protocol.hpp
    include/join/netlink_endpoint.hpp
    include/join/netlink_manager.hpp
    include/join/neighbor.hpp
    include/join/neighbor_manager.hpp
    include/join/arp.hpp
    include/join/interface.hpp
    include/join/interface_manager.hpp
    include/join/route.hpp
    include/join/route_manager.hpp
)

set(SOURCES
    src/netlink_manager.cpp
    src/neighbor.cpp
    src/neighbor_manager.cpp
    src/arp.cpp
    src/interface.cpp
    src/interface_manager.cpp
    src/route.cpp
    src/route_manager.cpp
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
        ${JOIN_CRYPTO}
        ${JOIN_CORE}
)

install(TARGETS ${JOIN_FABRIC}
    EXPORT joinTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/join
)

configure_file(${JOIN_FABRIC}.pc.in ${JOIN_FABRIC}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_FABRIC}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
