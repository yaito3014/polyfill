option(YK_POLYFILL_CI "for CI" OFF)

set(CMAKE_COLOR_DIAGNOSTICS ${YK_POLYFILL_CI})

if(DEFINED CMAKE_CXX_STANDARD)
    set(YK_POLYFILL_CXX_STANDARD ${CMAKE_CXX_STANDARD})
else()
    set(YK_POLYFILL_CXX_STANDARD 11)
endif()

# Fetches Catch2 in a function scope so that the variables set to configure it do not leak into the test suites.
function(yk_polyfill_fetch_catch2 cxx_standard out_major_version)
    include(FetchContent)
    if(${cxx_standard} LESS 14)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.10
            GIT_SHALLOW ON
        )

        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(CATCH_BUILD_STATIC_LIBRARY ON)
        FetchContent_MakeAvailable(Catch2)

        # Catch2 v2 uses std::nothrow without including <new>, which libc++ does not provide transitively.
        include(CheckCXXSourceCompiles)
        check_cxx_source_compiles(
            "#include <cstddef>\n#ifndef _LIBCPP_VERSION\n#error not libc++\n#endif\nint main() {}"
            YK_POLYFILL_STDLIB_IS_LIBCXX
        )
        if(YK_POLYFILL_STDLIB_IS_LIBCXX)
            target_compile_options(Catch2 INTERFACE "SHELL:-include new")
        endif()

        set(${out_major_version} 2 PARENT_SCOPE)
    else()
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.11.0
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(Catch2)

        set(${out_major_version} 3 PARENT_SCOPE)
    endif()
endfunction()

set(YK_POLYFILL_CXX_VERSIONS 11 14 17 20 23 26)

foreach(required_cxx_version ${YK_POLYFILL_CXX_VERSIONS})
    if(${required_cxx_version} GREATER ${YK_POLYFILL_CXX_STANDARD})
        continue()
    endif()

    yk_polyfill_fetch_catch2(${YK_POLYFILL_CXX_STANDARD} YK_POLYFILL_CATCH2_MAJOR_VERSION)

    set_target_properties(Catch2WithMain PROPERTIES CXX_EXTENSIONS OFF)

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cxx${required_cxx_version})
        add_subdirectory(cxx${required_cxx_version})
    endif()
endforeach()
