add_executable(yk_polyfill_cxx11_test)

target_sources(yk_polyfill_cxx11_test PRIVATE integer_sequence.cpp)

target_link_libraries(
    yk_polyfill_cxx11_test
    PRIVATE yk::polyfill::cxx11 Catch2::Catch2WithMain
)
