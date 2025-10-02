#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/is_unbounded_array.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_unbounded_array")
{
  STATIC_REQUIRE(pf::is_unbounded_array<int[]>::value == true);
  STATIC_REQUIRE(pf::is_unbounded_array<int[1]>::value == false);
  STATIC_REQUIRE(pf::is_unbounded_array<int*>::value == false);
  STATIC_REQUIRE(pf::is_unbounded_array<int>::value == false);
}
