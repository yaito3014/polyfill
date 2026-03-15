#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/extension/toptional.hpp>

namespace pf = yk::polyfill;
namespace ext = pf::extension;

// Custom trait for testing
struct custom_traits {
  static constexpr bool is_engaged(int x) noexcept { return x != -1; }
  static constexpr int tombstone_value() noexcept { return -1; }
};

TEST_CASE("toptional constexpr C++17")
{
  // C++17: operator-> tests (CXX17_CONSTEXPR)
  {
    constexpr auto test_arrow_lvalue = []() {
      struct Point {
        int x, y;
        constexpr Point(int a, int b) : x(a), y(b) {}
      };

      struct point_traits {
        static constexpr bool is_engaged(Point const& p) noexcept { return p.x != -1 && p.y != -1; }
        static constexpr Point tombstone_value() noexcept { return Point{-1, -1}; }
      };

      ext::toptional<Point, point_traits> opt(pf::in_place, 10, 20);
      return opt->x == 10 && opt->y == 20;
    };
    STATIC_REQUIRE(test_arrow_lvalue());

    constexpr auto test_arrow_const = []() {
      struct Point {
        int x, y;
        constexpr Point(int a, int b) : x(a), y(b) {}
      };

      struct point_traits {
        static constexpr bool is_engaged(Point const& p) noexcept { return p.x != -1 && p.y != -1; }
        static constexpr Point tombstone_value() noexcept { return Point{-1, -1}; }
      };

      ext::toptional<Point, point_traits> const opt(pf::in_place, 10, 20);
      return opt->x == 10 && opt->y == 20;
    };
    STATIC_REQUIRE(test_arrow_const());
  }

  // C++17: Iterator operations (CXX17_CONSTEXPR)
  {
    // Basic iterator tests
    constexpr auto test_iterator_engaged = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      auto it = opt.begin();
      return it != opt.end() && *it == 42;
    };
    STATIC_REQUIRE(test_iterator_engaged());

    constexpr auto test_iterator_empty = []() {
      ext::toptional<int> opt;
      return opt.begin() == opt.end();
    };
    STATIC_REQUIRE(test_iterator_empty());

    constexpr auto test_iterator_operations = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      auto it = opt.begin();
      auto end = opt.end();
      return (end - it) == 1 && (it + 1) == end;
    };
    STATIC_REQUIRE(test_iterator_operations());

    // Iterator arrow operator tests
    constexpr auto test_iterator_arrow = []() {
      struct Point {
        int x, y;
        constexpr Point(int a, int b) : x(a), y(b) {}
      };

      struct point_traits {
        static constexpr bool is_engaged(Point const& p) noexcept { return p.x != -1 && p.y != -1; }
        static constexpr Point tombstone_value() noexcept { return Point{-1, -1}; }
      };

      ext::toptional<Point, point_traits> opt(pf::in_place, 10, 20);
      auto it = opt.begin();
      return it->x == 10 && it->y == 20;
    };
    STATIC_REQUIRE(test_iterator_arrow());

    // Range-based for loop tests
    constexpr auto test_iterator_range = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      int sum = 0;
      for (auto val : opt) {
        sum += val;
      }
      return sum == 42;
    };
    STATIC_REQUIRE(test_iterator_range());

    constexpr auto test_iterator_empty_range = []() {
      ext::toptional<int> opt;
      int sum = 0;
      for (auto val : opt) {
        sum += val;
      }
      return sum == 0;
    };
    STATIC_REQUIRE(test_iterator_empty_range());
  }

  // C++17: Monadic operations with constexpr lambdas (and_then, transform, or_else)
  {
    // and_then tests
    constexpr auto test_and_then_engaged = []() {
      ext::toptional<int> opt(pf::in_place, 21);
      auto result = opt.and_then([](int x) { return ext::toptional<int>{x * 2}; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_and_then_engaged());

    constexpr auto test_and_then_empty = []() {
      ext::toptional<int> opt;
      auto result = opt.and_then([](int x) { return ext::toptional<int>{x * 2}; });
      return !result.has_value();
    };
    STATIC_REQUIRE(test_and_then_empty());

    constexpr auto test_and_then_returns_empty = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      auto result = opt.and_then([](int) { return ext::toptional<int>{}; });
      return !result.has_value();
    };
    STATIC_REQUIRE(test_and_then_returns_empty());

    constexpr auto test_and_then_const = []() {
      ext::toptional<int> const opt(pf::in_place, 21);
      auto result = opt.and_then([](int x) { return ext::toptional<int>{x * 2}; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_and_then_const());

    constexpr auto test_and_then_rvalue = []() {
      auto result = ext::toptional<int>(pf::in_place, 21).and_then([](int x) { return ext::toptional<int>{x * 2}; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_and_then_rvalue());

    // transform tests
    constexpr auto test_transform_engaged = []() {
      ext::toptional<int> opt(pf::in_place, 21);
      auto result = opt.transform([](int x) { return x * 2; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_transform_engaged());

    constexpr auto test_transform_empty = []() {
      ext::toptional<int> opt;
      auto result = opt.transform([](int x) { return x * 2; });
      return !result.has_value();
    };
    STATIC_REQUIRE(test_transform_empty());

    constexpr auto test_transform_type_change = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      auto result = opt.transform([](int x) { return x > 0; });
      return result.has_value() && *result == true;
    };
    STATIC_REQUIRE(test_transform_type_change());

    constexpr auto test_transform_const = []() {
      ext::toptional<int> const opt(pf::in_place, 21);
      auto result = opt.transform([](int x) { return x * 2; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_transform_const());

    constexpr auto test_transform_rvalue = []() {
      auto result = ext::toptional<int>(pf::in_place, 21).transform([](int x) { return x * 2; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_transform_rvalue());

    constexpr auto test_transform_with_custom_traits = []() {
      ext::toptional<int> opt(pf::in_place, 5);
      auto result = opt.transform<custom_traits>([](int x) { return x * 2; });
      return result.has_value() && *result == 10;
    };
    STATIC_REQUIRE(test_transform_with_custom_traits());

    // or_else tests
    constexpr auto test_or_else_engaged = []() {
      ext::toptional<int> opt(pf::in_place, 42);
      auto result = opt.or_else([]() { return ext::toptional<int>{99}; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_or_else_engaged());

    constexpr auto test_or_else_empty = []() {
      ext::toptional<int> opt;
      auto result = opt.or_else([]() { return ext::toptional<int>{99}; });
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_or_else_empty());

    constexpr auto test_or_else_const = []() {
      ext::toptional<int> const opt;
      auto result = opt.or_else([]() { return ext::toptional<int>{99}; });
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_or_else_const());

    constexpr auto test_or_else_rvalue = []() {
      auto result = ext::toptional<int>().or_else([]() { return ext::toptional<int>{99}; });
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_or_else_rvalue());

    // Chaining monadic operations
    constexpr auto test_chaining = []() {
      ext::toptional<int> opt(pf::in_place, 5);
      auto result = opt.transform([](int x) { return x * 2; })                           // 10
                        .and_then([](int x) { return ext::toptional<int>{x + 32}; })     // 42
                        .transform([](int x) { return x / 2; });                         // 21
      return result.has_value() && *result == 21;
    };
    STATIC_REQUIRE(test_chaining());

    constexpr auto test_chaining_with_empty = []() {
      ext::toptional<int> opt;
      auto result = opt.transform([](int x) { return x * 2; })
                        .and_then([](int x) { return ext::toptional<int>{x + 32}; })
                        .or_else([]() { return ext::toptional<int>{99}; });
      return result.has_value() && *result == 99;
    };
    STATIC_REQUIRE(test_chaining_with_empty());
  }

  // C++17: Complex constexpr scenarios with lambdas
  {
    constexpr auto test_complex = []() {
      struct Data {
        int x, y;
        constexpr Data(int a, int b) : x(a), y(b) {}
      };

      struct data_traits {
        static constexpr bool is_engaged(Data const& d) noexcept { return d.x != -1 && d.y != -1; }
        static constexpr Data tombstone_value() noexcept { return Data{-1, -1}; }
      };

      ext::toptional<Data, data_traits> opt(pf::in_place, 10, 20);

      // Use transform to modify data
      auto result = opt.transform<data_traits>([](Data const& d) { return Data{d.x * 2, d.y * 2}; });

      return result.has_value() && result->x == 20 && result->y == 40;
    };
    STATIC_REQUIRE(test_complex());
  }

  // C++17: Pointer types with custom traits
  {
    constexpr auto test_pointer_traits = []() {
      struct ptr_traits {
        static constexpr bool is_engaged(int const* p) noexcept { return p != nullptr; }
        static constexpr int const* tombstone_value() noexcept { return nullptr; }
      };

      constexpr int value = 42;
      ext::toptional<int const*, ptr_traits> opt(&value);

      auto result = opt.transform([](int const* p) { return *p; });
      return result.has_value() && *result == 42;
    };
    STATIC_REQUIRE(test_pointer_traits());
  }
}
