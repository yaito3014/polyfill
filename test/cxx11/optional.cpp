#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/optional.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("optional")
{
  // triviality check
  STATIC_REQUIRE(std::is_trivially_copy_constructible<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_constructible<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_copy_assignable<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_assignable<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_destructible<pf::optional<int>>::value);

  {
    pf::optional<int> opt;
    CHECK(!opt.has_value());
  }

  {
    pf::optional<int> opt(pf::in_place_holder::value, 42);
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }
}
