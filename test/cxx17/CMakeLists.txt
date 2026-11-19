add_executable(yk_polyfill_cxx17_test)

target_sources(
    yk_polyfill_cxx17_test
    PRIVATE
        invocable_traits.cpp
        constant_wrapper.cpp
        optional.cpp
        toptional.cpp
        variant.cpp
        function_ref.cpp
        move_only_function.cpp
        copyable_function.cpp
        expected.cpp
        unique_ptr.cpp
        loose_swap.cpp
        span.cpp
)

set_target_properties(yk_polyfill_cxx17_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(
    yk_polyfill_cxx17_test
    PRIVATE YK_POLYFILL_CATCH2_MAJOR_VERSION=${YK_POLYFILL_CATCH2_MAJOR_VERSION}
)

target_compile_features(yk_polyfill_cxx17_test PRIVATE cxx_std_17)
target_link_libraries(
    yk_polyfill_cxx17_test
    PRIVATE yk::polyfill Catch2::Catch2WithMain
)

add_test(NAME yk_polyfill_cxx17_test COMMAND yk_polyfill_cxx17_test)

# Without the yk::polyfill target, which adds /Zc:__cplusplus, MSVC leaves __cplusplus at 199711L.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_library(yk_polyfill_cxx17_without_zc_cplusplus OBJECT without_zc_cplusplus.cpp)
    set_target_properties(yk_polyfill_cxx17_without_zc_cplusplus PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(yk_polyfill_cxx17_without_zc_cplusplus PRIVATE cxx_std_17)
    target_compile_options(yk_polyfill_cxx17_without_zc_cplusplus PRIVATE /Zc:__cplusplus-)
    target_include_directories(yk_polyfill_cxx17_without_zc_cplusplus PRIVATE ${PROJECT_SOURCE_DIR}/include)
endif()
