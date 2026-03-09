# AGENTS.md

## Build

```bash
cmake -B build -DCMAKE_CXX_STANDARD=20
cmake --build build
```

Any standard version (11–26) works; C++20 enables the full constexpr test suite.

## Test

```bash
ctest --test-dir build
```

Or with verbose output on failure:

```bash
ctest --test-dir build --output-on-failure
```

## Test layout

| Directory          | Standard | What it tests |
|--------------------|----------|---------------|
| `test/cxx11/`      | C++11    | Runtime semantics — construction, copy, move, swap, equality, **allocator propagation** |
| `test/cxx14/`      | C++14    | C++14-specific utilities |
| `test/cxx17/`      | C++17    | C++17-specific utilities |
| `test/cxx20/`      | C++20    | `STATIC_REQUIRE` constexpr tests for `indirect` and `polymorphic` |

## Guidance for indirect / polymorphic changes

- **Allocator-propagation behavior** (POCS / POCCA / POCMA / `is_always_equal`) is
  tested in `test/cxx11/indirect.cpp` and `test/cxx11/polymorphic.cpp` using
  `TestAlloc<T, Pocs, Pocca, Pocma>`, a minimal stateful allocator defined at the
  top of the allocator-propagation section in each file.
- **Constexpr correctness** (operations that do not touch the allocator are
  `YK_POLYFILL_CXX14_CONSTEXPR`; allocating/deallocating operations are
  `YK_POLYFILL_CXX20_CONSTEXPR`) is verified via `STATIC_REQUIRE` in
  `test/cxx20/indirect.cpp` and `test/cxx20/polymorphic.cpp`.
- The allocator-ops dispatch uses **class template specialization** (not tag
  dispatch) — five helper structs per type (`swap_ops`, `copy_assign_ops`,
  `move_assign_ops`, `move_assign_ne_ops`, `move_ctor_ops`) live in
  `indirect_detail` / `polymorphic_detail` and are defined after the owning
  class is complete so they can access private members via `friend` declarations.
