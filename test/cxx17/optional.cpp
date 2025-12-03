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

TEST_CASE("optional constexpr C++17")
{
  // C++17: Iterator operations (require operator->() which is CXX17_CONSTEXPR)
  {
    constexpr auto test_iterators = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto it = opt.begin();
      return it != opt.end() && *it == 42 && (opt.end() - opt.begin()) == 1;
    };
    STATIC_REQUIRE(test_iterators());

    constexpr auto test_empty_iterators = []() {
      pf::optional<int> opt;
      return opt.begin() == opt.end() && (opt.end() - opt.begin()) == 0;
    };
    STATIC_REQUIRE(test_empty_iterators());

    constexpr auto test_iterator_increment = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto it = opt.begin();
      ++it;
      return it == opt.end();
    };
    STATIC_REQUIRE(test_iterator_increment());

    constexpr auto test_iterator_decrement = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto it = opt.end();
      --it;
      return it == opt.begin() && *it == 42;
    };
    STATIC_REQUIRE(test_iterator_decrement());
  }

  // C++17: Monadic operations are constexpr (require constexpr lambdas)
  {
    // and_then with engaged optional
    constexpr auto test_and_then_engaged = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = [](int x) { return pf::optional<int>(pf::in_place_holder::value, x * 2); };
      auto result = opt.and_then(f);
      return result.has_value() && *result == 84;
    };
    STATIC_REQUIRE(test_and_then_engaged());

    // and_then with empty optional
    constexpr auto test_and_then_empty = []() {
      pf::optional<int> opt;
      auto f = [](int x) { return pf::optional<int>(pf::in_place_holder::value, x * 2); };
      auto result = opt.and_then(f);
      return !result.has_value();
    };
    STATIC_REQUIRE(test_and_then_empty());

    // and_then returning empty
    constexpr auto test_and_then_return_empty = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = [](int) { return pf::optional<int>(); };
      auto result = opt.and_then(f);
      return !result.has_value();
    };
    STATIC_REQUIRE(test_and_then_return_empty());

    // and_then with type conversion
    constexpr auto test_and_then_convert = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = [](int x) { return pf::optional<double>(pf::in_place_holder::value, x * 2.5); };
      auto result = opt.and_then(f);
      return result.has_value() && *result == 105.0;
    };
    STATIC_REQUIRE(test_and_then_convert());
  }

  // C++17: transform operations
  {
    // transform with engaged optional
    constexpr auto test_transform_engaged = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = [](int x) { return x * 2; };
      auto result = opt.transform(f);
      return result.has_value() && *result == 84;
    };
    STATIC_REQUIRE(test_transform_engaged());

    // transform with empty optional
    constexpr auto test_transform_empty = []() {
      pf::optional<int> opt;
      auto f = [](int x) { return x * 2; };
      auto result = opt.transform(f);
      return !result.has_value();
    };
    STATIC_REQUIRE(test_transform_empty());

    // transform with type conversion
    constexpr auto test_transform_convert = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = [](int x) { return static_cast<double>(x) * 2.5; };
      auto result = opt.transform(f);
      return result.has_value() && *result == 105.0;
    };
    STATIC_REQUIRE(test_transform_convert());
  }

  // C++17: or_else operations
  {
    // or_else with engaged optional
    constexpr auto test_or_else_engaged = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto f = []() { return pf::optional<int>(pf::in_place_holder::value, 99); };
      auto result = opt.or_else(f);
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_or_else_engaged());

    // or_else with empty optional
    constexpr auto test_or_else_empty = []() {
      pf::optional<int> opt;
      auto f = []() { return pf::optional<int>(pf::in_place_holder::value, 99); };
      auto result = opt.or_else(f);
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_or_else_empty());

    // or_else returning empty
    constexpr auto test_or_else_return_empty = []() {
      pf::optional<int> opt;
      auto f = []() { return pf::optional<int>(); };
      auto result = opt.or_else(f);
      return !result.has_value();
    };
    STATIC_REQUIRE(test_or_else_return_empty());
  }

  // C++17: Chaining monadic operations
  {
    constexpr auto test_chain = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 10);
      auto result = opt
        .transform([](int x) { return x * 2; })
        .and_then([](int x) { return pf::optional<int>(pf::in_place_holder::value, x + 5); })
        .transform([](int x) { return x * 3; });
      return result.has_value() && *result == 75; // (10 * 2 + 5) * 3 = 75
    };
    STATIC_REQUIRE(test_chain());

    constexpr auto test_chain_with_empty = []() {
      pf::optional<int> opt;
      auto result = opt
        .transform([](int x) { return x * 2; })
        .or_else([]() { return pf::optional<int>(pf::in_place_holder::value, 100); })
        .transform([](int x) { return x + 50; });
      return result.has_value() && *result == 150;
    };
    STATIC_REQUIRE(test_chain_with_empty());
  }
}

TEST_CASE("optional<T&> constexpr C++17")
{
  // C++17: Monadic operations with references
  {
    // and_then with reference optional
    constexpr auto test_ref_and_then = []() {
      int x = 42;
      pf::optional<int&> opt(pf::in_place_holder::value, x);
      auto f = [](int& val) { return pf::optional<int>(pf::in_place_holder::value, val * 2); };
      auto result = opt.and_then(f);
      return result.has_value() && *result == 84;
    };
    STATIC_REQUIRE(test_ref_and_then());

    // transform with reference optional
    constexpr auto test_ref_transform = []() {
      int x = 42;
      pf::optional<int&> opt(pf::in_place_holder::value, x);
      auto f = [](int& val) { return val * 2; };
      auto result = opt.transform(f);
      return result.has_value() && *result == 84;
    };
    STATIC_REQUIRE(test_ref_transform());

    // or_else with reference optional
    constexpr auto test_ref_or_else = []() {
      int x = 99;
      pf::optional<int&> opt;
      auto f = [&]() { return pf::optional<int&>(pf::in_place_holder::value, x); };
      auto result = opt.or_else(f);
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_ref_or_else());
  }
}
