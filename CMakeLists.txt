cmake_minimum_required(VERSION 3.23)

project(yk_polyfill VERSION 0.0.0 LANGUAGES CXX)

set(YK_POLYFILL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/include/yk/polyfill)

include(GNUInstallDirs)

# common headers
add_library(yk_polyfill_common INTERFACE)
set_target_properties(
    yk_polyfill_common
    PROPERTIES EXPORT_NAME polyfill::common
)
if(MSVC)
    target_compile_options(yk_polyfill_common INTERFACE /Zc:__cplusplus)
endif()
file(GLOB YK_POLYFILL_COMMON_HEADERS ${YK_POLYFILL_PREFIX}/*.hpp)
target_sources(
    yk_polyfill_common
    INTERFACE
        FILE_SET yk_polyfill_common_headers_set
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES ${YK_POLYFILL_COMMON_HEADERS}
)

# versioned headers
list(
    APPEND
    YK_POLYFILL_CXX_VERSIONS
    11
    14
    17
    20
    23
    26
)

foreach(cxx_version ${YK_POLYFILL_CXX_VERSIONS})
    set(target_name yk_polyfill_cxx${cxx_version})
    set(subfolder cxx${cxx_version})
    add_library(${target_name} INTERFACE)
    set_target_properties(
        ${target_name}
        PROPERTIES EXPORT_NAME polyfill::cxx${cxx_version} CXX_EXTENSIONS OFF
    )
    target_compile_features(${target_name} INTERFACE cxx_std_${cxx_version})
    file(
        GLOB_RECURSE YK_POLYFILL_CXX${cxx_version}_HEADERS
        ${YK_POLYFILL_PREFIX}/${subfolder}/*.hpp
    )
    target_sources(
        ${target_name}
        INTERFACE
            FILE_SET yk_polyfill_cxx${cxx_version}_headers_set
                TYPE HEADERS
                BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
                FILES ${YK_POLYFILL_CXX${cxx_version}_HEADERS}
    )
    target_link_libraries(${target_name} INTERFACE yk_polyfill_common)
    list(APPEND YK_POLYFILL_VERSIONED_TARGETS ${target_name})
endforeach()

# aggregate
add_library(yk_polyfill INTERFACE)
set_target_properties(yk_polyfill PROPERTIES EXPORT_NAME polyfill)
target_link_libraries(yk_polyfill INTERFACE ${YK_POLYFILL_VERSIONED_TARGETS})

# alias
add_library(yk::polyfill::common ALIAS yk_polyfill_common)
foreach(cxx_version ${YK_POLYFILL_CXX_VERSIONS})
    add_library(
        yk::polyfill::cxx${cxx_version}
        ALIAS yk_polyfill_cxx${cxx_version}
    )
endforeach()
add_library(yk::polyfill ALIAS yk_polyfill)

# test
option(YK_POLYFILL_BUILD_TESTING "build tests" ${PROJECT_IS_TOP_LEVEL})
if(YK_POLYFILL_BUILD_TESTING)
    add_subdirectory(test)
endif()

# install
install(
    TARGETS yk_polyfill_common
    EXPORT yk-polyfill-export
    FILE_SET yk_polyfill_common_headers_set
)

foreach(cxx_version ${YK_POLYFILL_CXX_VERSIONS})
    set(target_name yk_polyfill_cxx${cxx_version})
    set(subfolder yk/polyfill/cxx${cxx_version})
    install(
        TARGETS ${target_name}
        EXPORT yk-polyfill-export
        FILE_SET yk_polyfill_cxx${cxx_version}_headers_set
    )
endforeach()
install(TARGETS yk_polyfill EXPORT yk-polyfill-export)

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
