cmake_minimum_required(VERSION 3.11)

project(yk_polyfill VERSION 0.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

# common headers
add_library(yk_polyfill_common INTERFACE)
target_include_directories(
    yk_polyfill_common
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
file(GLOB YK_POLYFILL_COMMON_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
target_sources(yk_polyfill_common INTERFACE ${YK_POLYFILL_COMMON_HEADERS})
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PREFIX yk_polyfill_common_headers
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
    set(subfolder yk/polyfill/cxx${cxx_version})
    add_library(${target_name} INTERFACE)
    target_include_directories(
        ${target_name}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    file(
        GLOB_RECURSE YK_POLYFILL_${cxx_version}_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${subfolder}/*.hpp
    )
    target_sources(
        ${target_name}
        INTERFACE ${YK_POLYFILL_${cxx_version}_HEADERS}
    )
    source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/${subfolder}
        PREFIX ${target_name}_headers
        FILES ${YK_POLYFILL_${cxx_version}_HEADERS}
    )
    target_link_libraries(${target_name} INTERFACE yk_polyfill_common)
    list(APPEND YK_POLYFILL_VERSIONED_TARGETS ${target_name})
endforeach()

# aggregate
add_library(yk_polyfill INTERFACE)
target_include_directories(
    yk_polyfill
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
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
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(YK_POLYFILL_BUILD_TESTING_DEFAULT ON)
else()
    set(YK_POLYFILL_BUILD_TESTING_DEFAULT OFF)
endif()
option(
    YK_POLYFILL_BUILD_TESTING
    "build tests"
    ${YK_POLYFILL_BUILD_TESTING_DEFAULT}
)
if(YK_POLYFILL_BUILD_TESTING)
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif()
