cmake_minimum_required(VERSION 3.14)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(PRIVATE_HEADER_DIRS
    core/src
    sax/src
)

set(PUBLIC_HEADER_DIRS
    core/include
    sax/include
)

set(PUBLIC_HEADERS
    core/include/join/error.hpp
    core/include/join/traits.hpp
    core/include/join/variant.hpp
    core/include/join/view.hpp
    sax/include/join/diyfp.hpp
    sax/include/join/dtoa.hpp
    sax/include/join/value.hpp
    sax/include/join/json.hpp
    sax/include/join/sax.hpp
)

set(SOURCES
    core/src/error.cpp
    sax/src/json.cpp
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
add_subdirectory(sax)

