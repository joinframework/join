cmake_minimum_required(VERSION 3.22.1)

find_package(ZLIB REQUIRED)

set(PUBLIC_HEADERS
    include/join/diyfp.hpp
    include/join/atodpow.hpp
    include/join/dtoapow.hpp
    include/join/dtoa.hpp
    include/join/view.hpp
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
add_library(join::data ALIAS ${JOIN_DATA})

set_target_properties(${JOIN_DATA} PROPERTIES
    VERSION ${JOIN_VERSION}
    SOVERSION ${JOIN_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_include_directories(${JOIN_DATA}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${JOIN_DATA}
    PUBLIC
        ${JOIN_CORE}
        ZLIB::ZLIB
)

install(TARGETS ${JOIN_DATA}
    EXPORT joinTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/join
)

configure_file(${JOIN_DATA}.pc.in ${JOIN_DATA}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JOIN_DATA}.pc DESTINATION lib/pkgconfig)

if(JOIN_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
