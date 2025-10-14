#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_bounded_array")
{
  STATIC_REQUIRE(pf::is_bounded_array<int[1]>::value == true);
  STATIC_REQUIRE(pf::is_bounded_array<int[]>::value == false);
  STATIC_REQUIRE(pf::is_bounded_array<int*>::value == false);
  STATIC_REQUIRE(pf::is_bounded_array<int>::value == false);
}
