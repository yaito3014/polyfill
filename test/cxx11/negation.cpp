#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

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
