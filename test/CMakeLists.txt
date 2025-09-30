enable_testing()

find_package(Catch2 3 CONFIG)

if(NOT Catch2_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
    )

    FetchContent_MakeAvailable(Catch2)
endif()

add_subdirectory(cxx11)
