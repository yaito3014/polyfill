#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/optional.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

// Helper constexpr functions for C++14 tests
// Note: In C++14, constexpr functions can modify local variables,
// but lambda expressions are not constexpr-capable until C++17

// value_or tests (CXX14_CONSTEXPR)
constexpr int test_value_or_engaged()
{
  pf::optional<int> opt(pf::in_place, 42);
  return opt.value_or(99);
}

constexpr int test_value_or_empty()
{
  pf::optional<int> opt;
  return opt.value_or(99);
}

// Iterator tests (CXX14_CONSTEXPR for begin/end and iterator operators)
// Note: These use operator->() internally which is CXX17_CONSTEXPR,
// so iterator tests are actually C++17+

// Dereference tests (CXX14_CONSTEXPR for & and &&)
constexpr bool test_dereference_lvalue()
{
  pf::optional<int> opt(pf::in_place, 42);
  return *opt == 42;
}

constexpr bool test_dereference_rvalue()
{
  pf::optional<int> opt(pf::in_place, 42);
  pf::optional<int> opt2 = std::move(opt);
  return *opt2 == 42;
}

}  // namespace

TEST_CASE("optional constexpr C++14")
{
  // C++14: value_or operation
  {
    STATIC_REQUIRE(test_value_or_engaged() == 42);
    STATIC_REQUIRE(test_value_or_empty() == 99);
  }

  // C++14: Dereference operations
  {
    STATIC_REQUIRE(test_dereference_lvalue());
    STATIC_REQUIRE(test_dereference_rvalue());
  }

  // Note: Monadic operations (and_then, transform, or_else) are marked as CXX14_CONSTEXPR
  // in the implementation, but they require constexpr lambdas which are only available in C++17+.
  // Therefore, those tests are in the C++17 test file.

  // Note: Iterator operations use operator->() which is CXX17_CONSTEXPR,
  // so iterator constexpr tests are in the C++17 test file.

  // Note: emplace, swap, reset, and assignment operations for optional<T> (non-reference)
  // are marked as CXX20_CONSTEXPR because they require constexpr destructors
  // which are only available in C++20+. Those tests are in the C++20 test file.
}

TEST_CASE("optional<T&> constexpr C++14")
{
  // Reference optional tests require C++17 for constexpr operator->() and constexpr lambdas
  // See C++17 test file for reference optional constexpr tests
}
