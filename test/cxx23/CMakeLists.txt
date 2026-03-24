add_executable(yk_polyfill_cxx23_test)

target_sources(yk_polyfill_cxx23_test PRIVATE constant_wrapper.cpp)

set_target_properties(yk_polyfill_cxx23_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(
    yk_polyfill_cxx23_test
    PRIVATE YK_POLYFILL_CATCH2_MAJOR_VERSION=${YK_POLYFILL_CATCH2_MAJOR_VERSION}
)

target_compile_features(yk_polyfill_cxx23_test PRIVATE cxx_std_23)
target_link_libraries(
    yk_polyfill_cxx23_test
    PRIVATE yk::polyfill Catch2::Catch2WithMain
)

add_test(NAME yk_polyfill_cxx23_test COMMAND yk_polyfill_cxx23_test)
