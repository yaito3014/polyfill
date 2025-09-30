cmake_minimum_required(VERSION 3.11)

project(polyfill VERSION 0.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

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

add_library(yk_polyfill_common INTERFACE)
file(GLOB YK_POLYFILL_COMMON_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
target_sources(yk_polyfill_common INTERFACE ${YK_POLYFILL_COMMON_HEADERS})

foreach(YK_POLYFILL_CXX_VERSION ${YK_POLYFILL_CXX_VERSIONS})
    set(YK_POLYFILL_LIBRARY yk_polyfill_cxx${YK_POLYFILL_CXX_VERSION})
    set(YK_POLYFILL_SUBFOLDER yk/polyfill/cxx${YK_POLYFILL_CXX_VERSION})

    add_library(${YK_POLYFILL_LIBRARY} INTERFACE)

    target_compile_features(
        ${YK_POLYFILL_LIBRARY}
        INTERFACE cxx_std_${YK_POLYFILL_CXX_VERSION}
    )

    target_include_directories(
        ${YK_POLYFILL_LIBRARY}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    file(
        GLOB_RECURSE YK_POLYFILL_CXX${YK_POLYFILL_CXX_VERSION}_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${YK_POLYFILL_SUBFOLDER}/*.hpp
    )

    source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/${YK_POLYFILL_SUBFOLDER}
        PREFIX ${YK_POLYFILL_LIBRARY}_headers
        FILES ${YK_POLYFILL_CXX${YK_POLYFILL_CXX_VERSION}_HEADERS}
    )

    target_sources(
        ${YK_POLYFILL_LIBRARY}
        INTERFACE ${YK_POLYFILL_CXX${YK_POLYFILL_CXX_VERSION}_HEADERS}
    )

    target_link_libraries(${YK_POLYFILL_LIBRARY} INTERFACE yk_polyfill_common)

    list(APPEND YK_POLYFILL_LIBRARIES ${YK_POLYFILL_LIBRARY})
endforeach()

add_library(yk_polyfill INTERFACE)

target_link_libraries(yk_polyfill INTERFACE ${YK_POLYFILL_LIBRARIES})

add_library(yk::polyfill ALIAS yk_polyfill)
foreach(YK_POLYFILL_CXX_VERSION ${YK_POLYFILL_CXX_VERSIONS})
    add_library(
        yk::polyfill::cxx${YK_POLYFILL_CXX_VERSION}
        ALIAS yk_polyfill_cxx${YK_POLYFILL_CXX_VERSION}
    )
endforeach()

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
