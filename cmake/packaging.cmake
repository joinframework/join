set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "lib${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${JOIN_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${JOIN_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${JOIN_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${JOIN_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "Mathieu Rabine")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modular C++ network runtime framework for Linux")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION
"join provides core networking, concurrency, serialization, cryptography and
Linux network fabric management.
This package contains the shared libraries.")
set(CPACK_COMPONENT_DEV_DESCRIPTION
"join provides core networking, concurrency, serialization, cryptography and
Linux network fabric management.
This package contains the headers, CMake configuration and pkg-config files.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/joinframework/join")

set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime dev)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "lib${PROJECT_NAME}${JOIN_VERSION_MAJOR}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ">=")

set(JOIN_DEV_DEPENDS "lib${PROJECT_NAME}${JOIN_VERSION_MAJOR} (= ${JOIN_VERSION})")
if(JOIN_ENABLE_CRYPTO)
    string(APPEND JOIN_DEV_DEPENDS ", libssl-dev")
endif()
if(JOIN_ENABLE_DATA)
    string(APPEND JOIN_DEV_DEPENDS ", zlib1g-dev")
endif()
if(JOIN_ENABLE_IO_URING)
    string(APPEND JOIN_DEV_DEPENDS ", liburing-dev")
endif()
if(JOIN_ENABLE_XDP)
    string(APPEND JOIN_DEV_DEPENDS ", libxdp-dev")
endif()
if(JOIN_ENABLE_NUMA)
    string(APPEND JOIN_DEV_DEPENDS ", libnuma-dev")
endif()

set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}-dev")
set(CPACK_DEBIAN_DEV_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "${JOIN_DEV_DEPENDS}")

include(CPack)
