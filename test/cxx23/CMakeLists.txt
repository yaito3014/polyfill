add_executable(yk_polyfill_cxx23_test)

set_target_properties(yk_polyfill_cxx23_test PROPERTIES CXX_STANDARD 23)

target_sources(yk_polyfill_cxx23_test PRIVATE constant_wrapper.cpp)

target_link_libraries(
    yk_polyfill_cxx23_test
    PRIVATE yk::polyfill::cxx23 Catch2::Catch2WithMain
)
