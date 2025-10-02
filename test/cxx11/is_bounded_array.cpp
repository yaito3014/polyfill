#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/is_bounded_array.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_bounded_array")
{
  STATIC_CHECK(pf::is_bounded_array<int[1]>::value == true);
  STATIC_CHECK(pf::is_bounded_array<int[]>::value == false);
  STATIC_CHECK(pf::is_bounded_array<int*>::value == false);
  STATIC_CHECK(pf::is_bounded_array<int>::value == false);
}
