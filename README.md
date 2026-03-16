# yk-polyfill

A header-only C++ library that backports standard library features to older C++ standards.

All headers are included via `<yk/polyfill/...>`. Everything lives in the `yk::polyfill` namespace (extensions in `yk::polyfill::extension`).

## Features

### Standard Library Polyfills

| Header | Provides |
|--------|----------|
| `type_traits.hpp` | `void_t`, `bool_constant`, `conjunction`, `disjunction`, `negation`, `remove_cvref`, `type_identity`, `is_bounded_array`, `is_unbounded_array`, `is_null_pointer`, `is_swappable`, `is_nothrow_convertible`, `constant_wrapper` (requires C++20) |
| `functional.hpp` | `invoke`, `invoke_r`, `is_invocable`, `is_nothrow_invocable`, `is_invocable_r`, `is_nothrow_invocable_r`, `invoke_result` |
| `utility.hpp` | `in_place_t`, `integer_sequence`, `make_index_sequence`, `exchange`, `as_const` |
| `memory.hpp` | `make_unique`, `unique_ptr`, `construct_at` |
| `tuple.hpp` | `apply` |
| `optional.hpp` | `optional` with monadic operations and iterator support |
| `variant.hpp` | `variant`, `visit`, `monostate` |
| `bit.hpp` | `bit_cast` |
| `indirect.hpp` | `indirect<T, A>`, value-semantic heap-allocated wrapper (C++26) |
| `polymorphic.hpp` | `polymorphic<T, A>`, value-semantic type-erased wrapper (C++26) |

### Extensions (`yk::polyfill::extension`)

| Header | Provides |
|--------|----------|
| `toptional.hpp` | `toptional<T, Traits>`, traits-customizable optional |
| `is_convertible_without_narrowing.hpp` | `is_convertible_without_narrowing<From, To>` |
| `specialization_of.hpp` | `is_specialization_of<T, Template>` |
| `pack_indexing.hpp` | `pack_indexing<I, Ts...>` |
| `always_false.hpp` | `always_false<Ts...>` |
| `ebo_storage.hpp` | `ebo_storage<T>` |

## Requirements

- CMake 3.23+
- C++11 or later

## Usage

### CMake

```cmake
add_subdirectory(polyfill)
target_link_libraries(your_target PRIVATE yk::polyfill::cxx11)  # or cxx14, cxx17, cxx20, cxx23, cxx26
```

### Code

```cpp
#include <yk/polyfill/optional.hpp>
#include <yk/polyfill/functional.hpp>

namespace pf = yk::polyfill;

pf::optional<int> opt = 42;
auto result = opt.and_then([](int x) -> pf::optional<int> { return x * 2; });
```

## Building and Testing

Tests are organized by the minimum C++ standard required to compile them (`test/cxx11/`, `test/cxx14/`, etc.). Which test suites are built is controlled by `CMAKE_CXX_STANDARD`:

```bash
# Build and run only cxx11 tests (default when CMAKE_CXX_STANDARD is not set)
cmake -B build-cxx11 -DCMAKE_CXX_STANDARD=11
cmake --build build-cxx11
ctest --test-dir build-cxx11

# Build and run all test suites (cxx11 through cxx26)
cmake -B build-cxx26 -DCMAKE_CXX_STANDARD=26
cmake --build build-cxx26
ctest --test-dir build-cxx26
```

A build with `-DCMAKE_CXX_STANDARD=N` includes only test suites whose required standard is `<= N`. Catch2 v2 is used for C++11 builds; Catch2 v3 is used for C++14 and later.

## CI

Tested across:
- **Compilers**: GCC 14, Clang 21, MSVC 2022/2026
- **Standards**: C++11, 14, 17, 20, 23, 26
- **Platforms**: Ubuntu 24.04, Windows

## Contributing

- Every non-trivial change should include a test that would fail if the change were reverted.
- Place tests in the appropriate `test/cxxN/` directory based on the minimum C++ standard they require.
- For new `constexpr` guarantees, add or extend a `STATIC_REQUIRE` check in the relevant test suite.
- Build and test against all supported standards (C++11, 14, 17, 20, 23, 26) before submitting.
- AI-generated code is accepted, but the author is responsible for its quality and correctness. "The AI wrote it" is not an excuse for bugs or poor code. Neither humans nor AI are perfect. Mistakes happen, and that's fine. When issues are found, fix them constructively rather than blaming the tool or the person.

## AI Usage

Unfortunately for some people, this project is not AI-free. AI tools are used for writing code and tests. That said, the main author puts effort into reviewing all changes for correctness and ensuring they meet the project's standards.

## License

[MIT](LICENSE) — Copyright (c) 2025 Yaito Kakeyama
