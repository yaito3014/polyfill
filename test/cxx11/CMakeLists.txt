add_executable(yk_polyfill_cxx11_test)

target_sources(
    yk_polyfill_cxx11_test
    PRIVATE
        integer_sequence.cpp
        integral_constant.cpp
        is_null_pointer.cpp
        is_bounded_array.cpp
        is_unbounded_array.cpp
        is_swappable_with.cpp
        is_swappable.cpp
        disjunction.cpp
        conjunction.cpp
        void_t.cpp
        remove_cvref.cpp
        make_unique.cpp
        negation.cpp
        invoke.cpp
        apply.cpp
        exchange.cpp
        reference_from_temporary.cpp
)

set_target_properties(yk_polyfill_cxx11_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(
    yk_polyfill_cxx11_test
    PRIVATE YK_POLYFILL_CATCH2_MAJOR_VERSION=${YK_POLYFILL_CATCH2_MAJOR_VERSION}
)

target_link_libraries(
    yk_polyfill_cxx11_test
    PRIVATE yk::polyfill::cxx11 Catch2::Catch2WithMain
)

add_test(NAME yk_polyfill_cxx11_test COMMAND yk_polyfill_cxx11_test)
