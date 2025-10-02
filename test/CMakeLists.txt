enable_testing()

find_package(Catch2 2 CONFIG)

if(NOT Catch2_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
    )

    set(CATCH_BUILD_STATIC_LIBRARY ON)
    FetchContent_MakeAvailable(Catch2)
endif()

add_subdirectory(cxx11)
add_subdirectory(cxx23)
