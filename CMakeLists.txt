cmake_minimum_required(VERSION 3.11)

project(polyfill VERSION 0.0.0)

add_library(polyfill INTERFACE)

target_compile_features(polyfill INTERFACE cxx_std_11)

target_include_directories(
    polyfill
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE YK_POLYFILL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PREFIX yk_polyfill_headers
    FILES ${YK_POLYFILL_HEADERS}
)
target_sources(polyfill INTERFACE ${YK_POLYFILL_HEADERS})

add_library(yk::polyfill ALIAS polyfill)

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
