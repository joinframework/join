cmake_minimum_required(VERSION 3.22.1)

set(PUBLIC_HEADERS
    include/join/dnsmessage.hpp
    include/join/resolver.hpp
    include/join/nameserver.hpp
    include/join/netlinkmanager.hpp
    include/join/neighbor.hpp
    include/join/neighbormanager.hpp
    include/join/arp.hpp
    include/join/interface.hpp
    include/join/interfacemanager.hpp
    include/join/route.hpp
    include/join/routemanager.hpp
)

set(SOURCES
    src/netlinkmanager.cpp
    src/neighbor.cpp
    src/neighbormanager.cpp
    src/arp.cpp
    src/interface.cpp
    src/interfacemanager.cpp
    src/route.cpp
    src/routemanager.cpp
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
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/join
)

configure_file(${JOIN_FABRIC}.pc.in ${JOIN_FABRIC}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_FABRIC}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
