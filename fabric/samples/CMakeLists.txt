cmake_minimum_required(VERSION 3.22.1)

add_executable(ifplugd ifplugd.cpp)
target_link_libraries(ifplugd ${JOIN_FABRIC})
install(TARGETS ifplugd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(ping ping.cpp)
target_link_libraries(ping ${JOIN_FABRIC})
install(TARGETS ping RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(traceroute traceroute.cpp)
target_link_libraries(traceroute ${JOIN_FABRIC})
install(TARGETS traceroute RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
