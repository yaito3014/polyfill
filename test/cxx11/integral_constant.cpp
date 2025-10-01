#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("integral_constant")
{
  {
    using IC = pf::integral_constant<int, 42>;
    STATIC_CHECK(IC::value == 42);
    STATIC_CHECK(std::is_same_v<IC::type, IC>);
    STATIC_CHECK(std::is_same_v<IC::value_type, int>);
    IC ic;
    STATIC_CHECK(ic == 42);
    STATIC_CHECK(ic() == 42);
  }
  {
    using BC = pf::bool_constant<true>;
    STATIC_CHECK(BC::value == true);
    STATIC_CHECK(std::is_same_v<BC::type, BC>);
    STATIC_CHECK(std::is_same_v<BC::value_type, bool>);
    BC bc;
    STATIC_CHECK(bc == true);
    STATIC_CHECK(bc() == true);
  }
}
