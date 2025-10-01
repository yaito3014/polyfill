#include <catch2/catch_test_macros.hpp>

#define YK_POLYFILL_NO_STD_CONSTANT_WRAPPER
#include <yk/polyfill/cxx23/constant_wrapper.hpp>

namespace pf = yk::polyfill;

TEST_CASE("constant_wrapper")
{
  STATIC_CHECK(pf::cw<12> + pf::cw<34> == pf::cw<46>);
  STATIC_CHECK(pf::cw<12> - pf::cw<34> == pf::cw<-22>);
  STATIC_CHECK(pf::cw<12> * pf::cw<34> == pf::cw<408>);
  STATIC_CHECK(pf::cw<12> / pf::cw<34> == pf::cw<0>);
  STATIC_CHECK(pf::cw<12> % pf::cw<34> == pf::cw<12>);

  // TODO: more tests
}
