include(CTest)

if(DEFINED CMAKE_CXX_STANDARD)
    set(YK_POLYFILL_CXX_STANDARD ${CMAKE_CXX_STANDARD})
else()
    set(YK_POLYFILL_CXX_STANDARD 11)
endif()

foreach(required_cxx_version ${YK_POLYFILL_CXX_VERSIONS})
    if(${required_cxx_version} GREATER ${YK_POLYFILL_CXX_STANDARD})
        continue()
    endif()

    include(FetchContent)
    if(${YK_POLYFILL_CXX_STANDARD} LESS 14)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.10
            GIT_SHALLOW ON
        )

        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(CATCH_BUILD_STATIC_LIBRARY ON)
        FetchContent_MakeAvailable(Catch2)

        set(YK_POLYFILL_CATCH2_MAJOR_VERSION 2)
    else()
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.11.0
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(Catch2)

        set(YK_POLYFILL_CATCH2_MAJOR_VERSION 3)
    endif()

    set_target_properties(Catch2WithMain PROPERTIES CXX_EXTENSIONS OFF)

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cxx${required_cxx_version})
        add_subdirectory(cxx${required_cxx_version})
    endif()
endforeach()
