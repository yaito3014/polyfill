#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

namespace pf = yk::polyfill;

TEST_CASE("variant constexpr get_if")
{
  constexpr pf::variant<int, double> v1 = 42;
  constexpr pf::variant<int, double> v2 = 3.14;

  // get_if<I>
  STATIC_REQUIRE(*pf::get_if<0>(&v1) == 42);
  STATIC_REQUIRE(pf::get_if<1>(&v1) == nullptr);

  // get_if<T>
  STATIC_REQUIRE(*pf::get_if<int>(&v1) == 42);
  STATIC_REQUIRE(pf::get_if<double>(&v1) == nullptr);

  STATIC_REQUIRE(*pf::get_if<1>(&v2) == 3.14);
  STATIC_REQUIRE(pf::get_if<0>(&v2) == nullptr);
}
