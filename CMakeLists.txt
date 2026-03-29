cmake_minimum_required(VERSION 3.23)

project(yk_polyfill VERSION 0.0.0 LANGUAGES CXX)

set(YK_POLYFILL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/include/yk/polyfill)

include(GNUInstallDirs)

# library
add_library(yk_polyfill INTERFACE)
set_target_properties(yk_polyfill PROPERTIES EXPORT_NAME polyfill)
if(MSVC)
    target_compile_options(yk_polyfill INTERFACE /Zc:__cplusplus)
endif()
file(GLOB_RECURSE YK_POLYFILL_HEADERS ${YK_POLYFILL_PREFIX}/*.hpp)
target_sources(
    yk_polyfill
    INTERFACE
        FILE_SET yk_polyfill_headers_set
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES ${YK_POLYFILL_HEADERS}
)

# alias
add_library(yk::polyfill ALIAS yk_polyfill)

# test
option(YK_POLYFILL_BUILD_TESTING "build tests" ${PROJECT_IS_TOP_LEVEL})

if(YK_POLYFILL_BUILD_TESTING AND NOT DEFINED BUILD_TESTING)
    set(BUILD_TESTING ON)
endif()

if(BUILD_TESTING)
    include(CTest)
    add_subdirectory(test)
endif()

# install
install(
    TARGETS yk_polyfill
    EXPORT yk-polyfill-export
    FILE_SET yk_polyfill_headers_set
)

install(
    EXPORT yk-polyfill-export
    FILE yk-polyfill-targets.cmake
    NAMESPACE yk::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yk/polyfill
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/yk-polyfill-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/yk-polyfill-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yk/polyfill
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/yk-polyfill-config-version.cmake
    VERSION 0.0.0
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/yk-polyfill-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/yk-polyfill-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yk/polyfill
)

export(
    EXPORT yk-polyfill-export
    NAMESPACE yk::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/yk-polyfill-targets.cmake
)
