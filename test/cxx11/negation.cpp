#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/negation.hpp>

struct true_type {
  static constexpr bool value = true;
};

struct false_type {
  static constexpr bool value = false;
};

namespace pf = yk::polyfill;

TEST_CASE("negation")
{
  STATIC_REQUIRE(pf::negation<true_type>::value == false);
  STATIC_REQUIRE(pf::negation<false_type>::value == true);
}
