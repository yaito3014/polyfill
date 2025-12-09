add_executable(yk_polyfill_cxx17_test)

target_sources(
    yk_polyfill_cxx17_test
    PRIVATE
        optional.cpp
        toptional.cpp
)

set_target_properties(yk_polyfill_cxx17_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(
    yk_polyfill_cxx17_test
    PRIVATE YK_POLYFILL_CATCH2_MAJOR_VERSION=${YK_POLYFILL_CATCH2_MAJOR_VERSION}
)

target_link_libraries(
    yk_polyfill_cxx17_test
    PRIVATE yk::polyfill::cxx17 Catch2::Catch2WithMain
)

add_test(NAME yk_polyfill_cxx17_test COMMAND yk_polyfill_cxx17_test)
