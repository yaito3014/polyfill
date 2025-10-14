#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

struct only_has_lhs_swap {
  template<class T>
  friend void swap(only_has_lhs_swap&, T&)
  {
  }
};

struct only_has_rhs_swap {
  template<class T>
  friend void swap(T&, only_has_rhs_swap&)
  {
  }
};

TEST_CASE("is_swappable_with")
{
  STATIC_REQUIRE(pf::is_swappable_with<int&, int&>::value == true);
  STATIC_REQUIRE(pf::is_swappable_with<int, int>::value == false);

  STATIC_REQUIRE(pf::is_swappable_with<only_has_lhs_swap&, int&>::value == false);
  STATIC_REQUIRE(pf::is_swappable_with<int&, only_has_rhs_swap&>::value == false);
}
