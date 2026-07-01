#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

namespace {

int fn(int x) { return 2 * x; }

}  // namespace

TEST_CASE("constant_wrapper")
{
  // arithmetic
  STATIC_REQUIRE(pf::cw<12> + pf::cw<34> == pf::cw<46>);
  STATIC_REQUIRE(pf::cw<12> - pf::cw<34> == pf::cw<-22>);
  STATIC_REQUIRE(pf::cw<12> * pf::cw<34> == pf::cw<408>);
  STATIC_REQUIRE(pf::cw<12> / pf::cw<34> == pf::cw<0>);
  STATIC_REQUIRE(pf::cw<12> % pf::cw<34> == pf::cw<12>);

  // bitwise
  STATIC_REQUIRE((pf::cw<5> & pf::cw<3>) == pf::cw<1>);
  STATIC_REQUIRE((pf::cw<5> | pf::cw<2>) == pf::cw<7>);
  STATIC_REQUIRE((pf::cw<5> ^ pf::cw<1>) == pf::cw<4>);

  // comparison
  STATIC_REQUIRE((pf::cw<12> < pf::cw<34>) == pf::cw<true>);
  STATIC_REQUIRE(pf::cw<12> == pf::cw<12>);
  STATIC_REQUIRE(pf::cw<12> != pf::cw<34>);

  // implicit conversion to the underlying value
  STATIC_REQUIRE(pf::cw<42> == 42);

  // pointer non-type template parameter (valid in C++17)
  STATIC_REQUIRE(pf::cw<&fn> == &fn);
}
