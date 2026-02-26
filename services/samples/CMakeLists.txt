cmake_minimum_required(VERSION 3.22.1)

add_executable(webbench webbench.cpp)
target_link_libraries(webbench ${JOIN_SERVICES})
install(TARGETS webbench RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
