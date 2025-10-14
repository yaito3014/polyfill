#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

TEST_CASE("constant_wrapper")
{
  STATIC_REQUIRE(pf::cw<12> + pf::cw<34> == pf::cw<46>);
  STATIC_REQUIRE(pf::cw<12> - pf::cw<34> == pf::cw<-22>);
  STATIC_REQUIRE(pf::cw<12> * pf::cw<34> == pf::cw<408>);
  STATIC_REQUIRE(pf::cw<12> / pf::cw<34> == pf::cw<0>);
  STATIC_REQUIRE(pf::cw<12> % pf::cw<34> == pf::cw<12>);

  // TODO: more tests
}
