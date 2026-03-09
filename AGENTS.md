# AGENTS.md

## Build and test

Configure, build, and test one directory per standard:

```bash
for std in 11 14 17 20; do
  cmake -B build-cxx${std} -DCMAKE_CXX_STANDARD=${std}
  cmake --build build-cxx${std}
  ctest --test-dir build-cxx${std} --output-on-failure
done
```

Each build-cxxNN directory includes only the test suites whose required standard
is ≤ N (enforced by `test/CMakeLists.txt`), so `build-cxx11` runs only the
`cxx11` suite while `build-cxx20` runs all four.

## Test layout

| Directory          | Standard | What it tests |
|--------------------|----------|---------------|
| `test/cxx11/`      | C++11    | Runtime semantics — construction, copy, move, swap, equality, **allocator propagation** |
| `test/cxx14/`      | C++14    | C++14-specific utilities |
| `test/cxx17/`      | C++17    | C++17-specific utilities |
| `test/cxx20/`      | C++20    | `STATIC_REQUIRE` constexpr tests for `indirect` and `polymorphic` |

## After making changes

For any non-trivial change, add a test that would fail if the change were reverted:

- **Runtime behavior change** → add a `TEST_CASE` to the appropriate `test/cxx11/`
  file (or `cxx17/` if it uses C++17 features).
- **New `constexpr` guarantee** (e.g. lowering `YK_POLYFILL_CXX20_CONSTEXPR` to
  `YK_POLYFILL_CXX14_CONSTEXPR`) → add or extend a `STATIC_REQUIRE` helper in
  `test/cxx20/` to exercise the operation at compile time.
- **Allocator-propagation path** → use `TestAlloc<T, Pocs, Pocca, Pocma>` (already
  defined in `test/cxx11/indirect.cpp` and `test/cxx11/polymorphic.cpp`) to
  construct objects with distinct allocator IDs and assert the expected post-state.
- Always run the full loop above and confirm all tests pass before committing.

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
