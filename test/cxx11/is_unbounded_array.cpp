#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/is_unbounded_array.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_unbounded_array")
{
  STATIC_CHECK(pf::is_unbounded_array<int[]>::value == true);
  STATIC_CHECK(pf::is_unbounded_array<int[1]>::value == false);
  STATIC_CHECK(pf::is_unbounded_array<int*>::value == false);
  STATIC_CHECK(pf::is_unbounded_array<int>::value == false);
}
