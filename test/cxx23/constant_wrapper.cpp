#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/type_traits.hpp>

#include <array>

namespace pf = yk::polyfill;

#if __cpp_multidimensional_subscript >= 202211L

struct SubscriptIsAdd {
  constexpr auto operator[](auto x, auto y) const { return x + y; }
};

#endif

TEST_CASE("constant_wrapper")
{
  STATIC_REQUIRE(pf::cw<12> + pf::cw<34> == pf::cw<46>);
  STATIC_REQUIRE(pf::cw<12> - pf::cw<34> == pf::cw<-22>);
  STATIC_REQUIRE(pf::cw<12> * pf::cw<34> == pf::cw<408>);
  STATIC_REQUIRE(pf::cw<12> / pf::cw<34> == pf::cw<0>);
  STATIC_REQUIRE(pf::cw<12> % pf::cw<34> == pf::cw<12>);

#if __cpp_explicit_this_parameter >= 202110L
  STATIC_REQUIRE(++pf::cw<12> == pf::cw<13>);
  STATIC_REQUIRE(pf::cw<std::array{33, 4}>[pf::cw<0>] == pf::cw<33>);
#endif

#if __cpp_multidimensional_subscript >= 202211L
  STATIC_REQUIRE(pf::cw<SubscriptIsAdd{}>[pf::cw<33>, pf::cw<4>] == pf::cw<37>);
#endif

  // TODO: more tests
}
