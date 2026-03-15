# AGENTS.md

## Build and test

Configure, build, and test one directory per standard:

```bash
for std in 11 14 17 20 23 26; do
  cmake -B build-cxx${std} -DCMAKE_CXX_STANDARD=${std}
  cmake --build build-cxx${std}
  ctest --test-dir build-cxx${std} --output-on-failure
done
```

Each build-cxxNN directory includes only the test suites whose required standard
is <= N (enforced by `test/CMakeLists.txt`), so `build-cxx11` runs only the
`cxx11` suite while `build-cxx26` runs all six.

## After making changes

For any non-trivial change, add a test that would fail if the change were reverted:

- **Runtime behavior change** -> add a `TEST_CASE` to the appropriate `test/cxx11/`
  file (or `cxx17/` if it uses C++17 features).
- **New `constexpr` guarantee** (e.g. lowering `YK_POLYFILL_CXX20_CONSTEXPR` to
  `YK_POLYFILL_CXX14_CONSTEXPR`) -> add or extend a `STATIC_REQUIRE` helper in
  `test/cxx20/` to exercise the operation at compile time.