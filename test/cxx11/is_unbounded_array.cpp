#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_unbounded_array")
{
  STATIC_REQUIRE(pf::is_unbounded_array<int[]>::value == true);
  STATIC_REQUIRE(pf::is_unbounded_array<int[1]>::value == false);
  STATIC_REQUIRE(pf::is_unbounded_array<int*>::value == false);
  STATIC_REQUIRE(pf::is_unbounded_array<int>::value == false);
}
