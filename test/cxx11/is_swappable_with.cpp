#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/is_swappable_with.hpp>

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
  STATIC_CHECK(pf::is_swappable_with<int&, int&>::value == true);
  STATIC_CHECK(pf::is_swappable_with<int, int>::value == false);

  STATIC_CHECK(pf::is_swappable_with<only_has_lhs_swap&, int&>::value == false);
  STATIC_CHECK(pf::is_swappable_with<int&, only_has_rhs_swap&>::value == false);
}
