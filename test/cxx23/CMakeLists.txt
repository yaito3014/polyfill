add_executable(yk_polyfill_cxx23_test)

target_sources(yk_polyfill_cxx23_test PRIVATE constant_wrapper.cpp)

set_target_properties(yk_polyfill_cxx23_test PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(
    yk_polyfill_cxx23_test
    PRIVATE yk::polyfill::cxx23 Catch2::Catch2WithMain
)

add_test(NAME yk_polyfill_cxx23_test COMMAND yk_polyfill_cxx23_test)
