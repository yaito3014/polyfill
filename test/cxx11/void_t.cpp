#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/void_t.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("void_t")
{
  STATIC_CHECK(std::is_same<pf::void_t<>, void>::value == true);
  STATIC_CHECK(std::is_same<pf::void_t<int>, void>::value == true);
  STATIC_CHECK(std::is_same<pf::void_t<int, int>, void>::value == true);
}
