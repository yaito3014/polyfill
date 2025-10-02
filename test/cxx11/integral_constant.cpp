#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("integral_constant")
{
  {
    using IC = pf::integral_constant<int, 42>;
    STATIC_REQUIRE(IC::value == 42);
    STATIC_REQUIRE(std::is_same<IC::type, IC>::value);
    STATIC_REQUIRE(std::is_same<IC::value_type, int>::value);
    IC ic;
    STATIC_REQUIRE(ic == 42);
    STATIC_REQUIRE(ic() == 42);
  }
  {
    using BC = pf::bool_constant<true>;
    STATIC_REQUIRE(BC::value == true);
    STATIC_REQUIRE(std::is_same<BC::type, BC>::value);
    STATIC_REQUIRE(std::is_same<BC::value_type, bool>::value);
    BC bc;
    STATIC_REQUIRE(bc == true);
    STATIC_REQUIRE(bc() == true);
  }
}
