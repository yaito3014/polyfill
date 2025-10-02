#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/is_bounded_array.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_bounded_array")
{
  STATIC_REQUIRE(pf::is_bounded_array<int[1]>::value == true);
  STATIC_REQUIRE(pf::is_bounded_array<int[]>::value == false);
  STATIC_REQUIRE(pf::is_bounded_array<int*>::value == false);
  STATIC_REQUIRE(pf::is_bounded_array<int>::value == false);
}
