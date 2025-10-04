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

    if(${required_cxx_version} LESS 14)
        find_package(Catch2 2 CONFIG)

        if(NOT Catch2_FOUND)
            include(FetchContent)

            FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v2.13.10
                GIT_SHALLOW ON
            )

            set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
            set(CATCH_BUILD_STATIC_LIBRARY ON)
            FetchContent_MakeAvailable(Catch2)
        endif()
    else()
        find_package(Catch2 3 CONFIG)

        if(NOT Catch2_FOUND)
            include(FetchContent)

            FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.11.0
                GIT_SHALLOW ON
            )

            FetchContent_MakeAvailable(Catch2)
        endif()
    endif()

    set_target_properties(Catch2WithMain PROPERTIES CXX_EXTENSIONS OFF)

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cxx${required_cxx_version})
        add_subdirectory(cxx${required_cxx_version})
    endif()
endforeach()
