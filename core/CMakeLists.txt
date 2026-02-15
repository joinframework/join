cmake_minimum_required(VERSION 3.22.1)

set(PUBLIC_HEADERS
    include/join/version.hpp
    include/join/error.hpp
    include/join/utils.hpp
    include/join/filesystem.hpp
    include/join/traits.hpp
    include/join/variant.hpp
    include/join/reactor.hpp
    include/join/reactorpool.hpp
    include/join/memory.hpp
    include/join/queue.hpp
    include/join/mutex.hpp
    include/join/condition.hpp
    include/join/semaphore.hpp
    include/join/thread.hpp
    include/join/threadpool.hpp
    include/join/macaddress.hpp
    include/join/ipaddress.hpp
    include/join/endpoint.hpp
    include/join/protocol.hpp
    include/join/socket.hpp
    include/join/socketstream.hpp
    include/join/acceptor.hpp
    include/join/openssl.hpp
)

set(SOURCES
    src/error.cpp
    src/reactor.cpp
    src/reactorpool.cpp
    src/mutex.cpp
    src/condition.cpp
    src/semaphore.cpp
    src/thread.cpp
    src/threadpool.cpp
    src/openssl.cpp
    src/macaddress.cpp
    src/ipaddress.cpp
    src/socket.cpp
)

add_library(${JOIN_CORE} ${SOURCES})
add_library(join::core ALIAS ${JOIN_CORE})

set_target_properties(${JOIN_CORE} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_include_directories(${JOIN_CORE}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_CORE}
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

install(TARGETS ${JOIN_CORE}
    EXPORT joinTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/join
)

configure_file(${JOIN_CORE}.pc.in ${JOIN_CORE}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_CORE}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
