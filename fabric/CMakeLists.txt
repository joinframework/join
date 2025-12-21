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
    include/join/resolver.hpp
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
set_target_properties(${JOIN_FABRIC} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${JOIN_FABRIC} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${JOIN_FABRIC} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${JOIN_FABRIC} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${JOIN_FABRIC} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

install(TARGETS ${JOIN_FABRIC}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_FABRIC}.pc.in ${JOIN_FABRIC}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_FABRIC}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
