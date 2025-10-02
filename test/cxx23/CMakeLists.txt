add_executable(yk_polyfill_cxx23_test)

target_sources(yk_polyfill_cxx23_test PRIVATE constant_wrapper.cpp)

target_link_libraries(
    yk_polyfill_cxx23_test
    PRIVATE yk::polyfill::cxx23 Catch2::Catch2WithMain
)
