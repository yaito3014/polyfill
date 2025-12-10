#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/toptional.hpp>
#include <yk/polyfill/utility.hpp>

namespace pf = yk::polyfill;
namespace ext = pf::extension;

// Helper constexpr functions for C++14 tests
// Note: In C++14, constexpr functions can modify local variables,
// but lambda expressions are not constexpr-capable until C++17

// Custom trait for testing
struct custom_traits {
  static constexpr bool is_engaged(int x) noexcept { return x != -1; }
  static constexpr int tombstone_value() noexcept { return -1; }
};

// Function objects for monadic operations (C++14 constexpr compatible)
struct double_value {
  constexpr int operator()(int x) const { return x * 2; }
};

struct safe_divide {
  constexpr ext::toptional<int> operator()(int x) const
  {
    if (x == 0) return ext::toptional<int>{};
    return ext::toptional<int>{100 / x};
  }
};

struct make_toptional_double {
  constexpr ext::toptional<int> operator()(int x) const { return ext::toptional<int>{x * 2}; }
};

struct return_empty_toptional {
  constexpr ext::toptional<int> operator()(int) const { return ext::toptional<int>{}; }
};

struct return_default_value {
  constexpr ext::toptional<int> operator()() const { return ext::toptional<int>{99}; }
};

struct is_positive {
  constexpr bool operator()(int x) const { return x > 0; }
};

// value_or tests (CXX14_CONSTEXPR)
constexpr int test_value_or_engaged()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return opt.value_or(99);
}

constexpr int test_value_or_empty()
{
  ext::toptional<int> opt;
  return opt.value_or(99);
}

constexpr int test_value_or_engaged_rvalue()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return ext::toptional<int>(std::move(opt)).value_or(99);
}

constexpr int test_value_or_empty_rvalue()
{
  ext::toptional<int> opt;
  return ext::toptional<int>(std::move(opt)).value_or(99);
}

// Dereference tests (CXX14_CONSTEXPR for & and &&)
constexpr bool test_dereference_lvalue()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return *opt == 42;
}

constexpr bool test_dereference_rvalue()
{
  ext::toptional<int> opt(pf::in_place, 42);
  ext::toptional<int> opt2 = std::move(opt);
  return *opt2 == 42;
}

constexpr bool test_dereference_const_lvalue()
{
  ext::toptional<int> const opt(pf::in_place, 42);
  return *opt == 42;
}

constexpr bool test_dereference_const_rvalue()
{
  ext::toptional<int> const opt(pf::in_place, 42);
  return *(ext::toptional<int>(opt)) == 42;
}

// Operator bool tests
constexpr bool test_operator_bool_engaged()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return static_cast<bool>(opt);
}

constexpr bool test_operator_bool_empty()
{
  ext::toptional<int> opt;
  return !static_cast<bool>(opt);
}

// has_value tests
constexpr bool test_has_value_engaged()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return opt.has_value();
}

constexpr bool test_has_value_empty()
{
  ext::toptional<int> opt;
  return !opt.has_value();
}

// Custom traits tests
constexpr bool test_custom_traits_engaged()
{
  ext::toptional<int, custom_traits> opt(pf::in_place, 0);  // 0 is valid with custom_traits
  return opt.has_value() && *opt == 0;
}

constexpr bool test_custom_traits_empty()
{
  ext::toptional<int, custom_traits> opt;  // Default constructs to -1 (tombstone)
  return !opt.has_value();
}

// Relational operators tests
constexpr bool test_equality_engaged()
{
  ext::toptional<int> opt1(pf::in_place, 42);
  ext::toptional<int> opt2(pf::in_place, 42);
  return opt1 == opt2;
}

constexpr bool test_equality_empty()
{
  ext::toptional<int> opt1;
  ext::toptional<int> opt2;
  return opt1 == opt2;
}

constexpr bool test_inequality_different_values()
{
  ext::toptional<int> opt1(pf::in_place, 42);
  ext::toptional<int> opt2(pf::in_place, 99);
  return opt1 != opt2;
}

constexpr bool test_less_than()
{
  ext::toptional<int> opt1(pf::in_place, 42);
  ext::toptional<int> opt2(pf::in_place, 99);
  ext::toptional<int> opt_empty;
  return opt1 < opt2 && opt_empty < opt1;
}

constexpr bool test_comparison_with_nullopt()
{
  ext::toptional<int> opt(pf::in_place, 42);
  ext::toptional<int> opt_empty;
  return opt != pf::nullopt && opt_empty == pf::nullopt;
}

constexpr bool test_comparison_with_value()
{
  ext::toptional<int> opt(pf::in_place, 42);
  return opt == 42 && opt != 99 && opt < 50 && opt > 30;
}

// Monadic operations with function objects (C++14 constexpr compatible)
// Note: These use YK_POLYFILL_CXX14_CONSTEXPR but work with function objects, not lambdas

// and_then tests
constexpr bool test_and_then_engaged()
{
  ext::toptional<int> opt(pf::in_place, 21);
  auto result = opt.and_then(make_toptional_double{});
  return result.has_value() && *result == 42;
}

constexpr bool test_and_then_empty()
{
  ext::toptional<int> opt;
  auto result = opt.and_then(make_toptional_double{});
  return !result.has_value();
}

constexpr bool test_and_then_returns_empty()
{
  ext::toptional<int> opt(pf::in_place, 42);
  auto result = opt.and_then(return_empty_toptional{});
  return !result.has_value();
}

constexpr bool test_and_then_const()
{
  ext::toptional<int> const opt(pf::in_place, 21);
  auto result = opt.and_then(make_toptional_double{});
  return result.has_value() && *result == 42;
}

// transform tests
constexpr bool test_transform_engaged()
{
  ext::toptional<int> opt(pf::in_place, 21);
  auto result = opt.transform(double_value{});
  return result.has_value() && *result == 42;
}

constexpr bool test_transform_empty()
{
  ext::toptional<int> opt;
  auto result = opt.transform(double_value{});
  return !result.has_value();
}

constexpr bool test_transform_type_change()
{
  ext::toptional<int> opt(pf::in_place, 42);
  auto result = opt.transform(is_positive{});
  return result.has_value() && *result == true;
}

constexpr bool test_transform_const()
{
  ext::toptional<int> const opt(pf::in_place, 21);
  auto result = opt.transform(double_value{});
  return result.has_value() && *result == 42;
}

constexpr bool test_transform_with_custom_traits()
{
  ext::toptional<int> opt(pf::in_place, 5);
  auto result = opt.transform<custom_traits>(double_value{});
  return result.has_value() && *result == 10;
}

// or_else tests
constexpr bool test_or_else_engaged()
{
  ext::toptional<int> opt(pf::in_place, 42);
  auto result = opt.or_else(return_default_value{});
  return result.has_value() && *result == 42;
}

constexpr bool test_or_else_empty()
{
  ext::toptional<int> opt;
  auto result = opt.or_else(return_default_value{});
  return result.has_value() && *result == 99;
}

constexpr bool test_or_else_const()
{
  ext::toptional<int> const opt;
  auto result = opt.or_else(return_default_value{});
  return result.has_value() && *result == 99;
}

// Chaining monadic operations
constexpr bool test_chaining_monadic_ops()
{
  ext::toptional<int> opt(pf::in_place, 5);
  auto result = opt.transform(double_value{})  // 10
                    .and_then(safe_divide{});  // 100 / 10 = 10
  return result.has_value() && *result == 10;
}

constexpr bool test_chaining_with_or_else()
{
  ext::toptional<int> opt;
  auto result = opt.transform(double_value{}).and_then(safe_divide{}).or_else(return_default_value{});
  return result.has_value() && *result == 99;
}

TEST_CASE("toptional constexpr C++14")
{
  // C++14: value_or operation
  {
    STATIC_REQUIRE(test_value_or_engaged() == 42);
    STATIC_REQUIRE(test_value_or_empty() == 99);
    STATIC_REQUIRE(test_value_or_engaged_rvalue() == 42);
    STATIC_REQUIRE(test_value_or_empty_rvalue() == 99);
  }

  // C++14: Dereference operations
  {
    STATIC_REQUIRE(test_dereference_lvalue());
    STATIC_REQUIRE(test_dereference_rvalue());
    STATIC_REQUIRE(test_dereference_const_lvalue());
    STATIC_REQUIRE(test_dereference_const_rvalue());
  }

  // C++14: operator bool and has_value
  {
    STATIC_REQUIRE(test_operator_bool_engaged());
    STATIC_REQUIRE(test_operator_bool_empty());
    STATIC_REQUIRE(test_has_value_engaged());
    STATIC_REQUIRE(test_has_value_empty());
  }

  // C++14: Custom traits
  {
    STATIC_REQUIRE(test_custom_traits_engaged());
    STATIC_REQUIRE(test_custom_traits_empty());
  }

  // C++14: Relational operators
  {
    STATIC_REQUIRE(test_equality_engaged());
    STATIC_REQUIRE(test_equality_empty());
    STATIC_REQUIRE(test_inequality_different_values());
    STATIC_REQUIRE(test_less_than());
    STATIC_REQUIRE(test_comparison_with_nullopt());
    STATIC_REQUIRE(test_comparison_with_value());
  }

  // C++14: Monadic operations with function objects
  // Note: Monadic operations are marked as CXX14_CONSTEXPR in the implementation.
  // While constexpr lambdas require C++17+, we can use constexpr function objects in C++14.
  {
    // and_then
    STATIC_REQUIRE(test_and_then_engaged());
    STATIC_REQUIRE(test_and_then_empty());
    STATIC_REQUIRE(test_and_then_returns_empty());
    STATIC_REQUIRE(test_and_then_const());

    // transform
    STATIC_REQUIRE(test_transform_engaged());
    STATIC_REQUIRE(test_transform_empty());
    STATIC_REQUIRE(test_transform_type_change());
    STATIC_REQUIRE(test_transform_const());
    STATIC_REQUIRE(test_transform_with_custom_traits());

    // or_else
    STATIC_REQUIRE(test_or_else_engaged());
    STATIC_REQUIRE(test_or_else_empty());
    STATIC_REQUIRE(test_or_else_const());

    // Chaining
    STATIC_REQUIRE(test_chaining_monadic_ops());
    STATIC_REQUIRE(test_chaining_with_or_else());
  }

  // Note: swap, emplace, reset, and assignment operations for toptional
  // are marked as CXX20_CONSTEXPR because they require constexpr destructors
  // (and constexpr std::swap in the case of swap) which are only available in C++20+.
  // Those tests are in the C++20 test file.
}
