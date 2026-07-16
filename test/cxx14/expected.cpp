#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/expected.hpp>
#include <yk/polyfill/utility.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("expected constexpr C++14")
{
  // constexpr construction + observers (trivial arms)
  {
    constexpr pf::expected<int, int> e(pf::in_place, 5);
    STATIC_REQUIRE(e.has_value());
    STATIC_REQUIRE(static_cast<bool>(e));
    STATIC_REQUIRE(*e == 5);
    STATIC_REQUIRE(e.value() == 5);
    STATIC_REQUIRE(e.value_or(0) == 5);
    STATIC_REQUIRE(e.error_or(3) == 3);
  }
  {
    constexpr pf::expected<int, int> u(pf::unexpect, 9);
    STATIC_REQUIRE(!u.has_value());
    STATIC_REQUIRE(u.error() == 9);
    STATIC_REQUIRE(u.value_or(3) == 3);
    STATIC_REQUIRE(u.error_or(0) == 9);
  }

  // constexpr comparisons
  {
    constexpr pf::expected<int, int> a(pf::in_place, 5);
    constexpr pf::expected<int, int> b(pf::in_place, 5);
    constexpr pf::expected<int, int> e(pf::unexpect, 1);
    STATIC_REQUIRE(a == b);
    STATIC_REQUIRE(a != e);
    STATIC_REQUIRE(a == 5);
    STATIC_REQUIRE(e == pf::unexpected<int>(1));
  }

  // constexpr void specialization
  {
    constexpr pf::expected<void, int> v;
    STATIC_REQUIRE(v.has_value());
    constexpr pf::expected<void, int> u(pf::unexpect, 4);
    STATIC_REQUIRE(!u.has_value());
    STATIC_REQUIRE(u.error() == 4);
    STATIC_REQUIRE(v != u);
  }
}
