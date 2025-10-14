#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("void_t")
{
  STATIC_REQUIRE(std::is_same<pf::void_t<>, void>::value == true);
  STATIC_REQUIRE(std::is_same<pf::void_t<int>, void>::value == true);
  STATIC_REQUIRE(std::is_same<pf::void_t<int, int>, void>::value == true);
}
