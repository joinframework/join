cmake_minimum_required(VERSION 3.22.1)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

set(PRIVATE_HEADER_DIRS
    src
)

set(PUBLIC_HEADER_DIRS
    include
)

set(PUBLIC_HEADERS
    include/join/diyfp.hpp
    include/join/atodpow.hpp
    include/join/dtoapow.hpp
    include/join/dtoa.hpp
    include/join/value.hpp
    include/join/sax.hpp
    include/join/pack.hpp
    include/join/json.hpp
    include/join/zstream.hpp
)

set(SOURCES
    src/value.cpp
    src/sax.cpp
    src/json.cpp
    src/zstream.cpp
)

add_library(${JOIN_DATA} ${SOURCES})
set_target_properties(${JOIN_DATA} PROPERTIES VERSION ${JOIN_VERSION} SOVERSION ${JOIN_VERSION_MAJOR})
set_target_properties(${JOIN_DATA} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${JOIN_DATA} PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(${JOIN_DATA} PRIVATE ${PRIVATE_HEADER_DIRS})

target_link_libraries(${JOIN_DATA} ${JOIN_CORE} OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB Threads::Threads)

install(TARGETS ${JOIN_DATA}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_DATA}.pc.in ${JOIN_DATA}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_DATA}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
