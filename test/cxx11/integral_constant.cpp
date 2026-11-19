#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

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

TEST_CASE("integral_constant::value can be odr-used")
{
  // Taking the address odr-uses value, which needs a definition at namespace scope before C++17.
  int const* volatile p = &pf::integral_constant<int, 42>::value;
  CHECK(*p == 42);
  bool const* volatile b = &pf::true_type::value;
  CHECK(*b);
}
