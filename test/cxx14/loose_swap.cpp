#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/concepts.hpp>

namespace pf = yk::polyfill;

namespace {

struct constexpr_adl_swap {
  int value;
  bool via_adl;
  friend constexpr void swap(constexpr_adl_swap& a, constexpr_adl_swap& b) noexcept
  {
    int const tmp = a.value;
    a.value = b.value;
    b.value = tmp;
    a.via_adl = true;
    b.via_adl = true;
  }
};

constexpr bool test_adl()
{
  constexpr_adl_swap a{1, false}, b{2, false};
  pf::detail::loose_swap(a, b);
  return a.value == 2 && b.value == 1 && a.via_adl;
}

constexpr bool test_pointer()
{
  int x = 1, y = 2;
  int* p = &x;
  int* q = &y;
  pf::detail::loose_swap(p, q);
  return p == &y && q == &x;
}

constexpr bool test_array()
{
  int a[2] = {1, 2}, b[2] = {3, 4};
  pf::detail::loose_swap(a, b);
  return a[0] == 3 && a[1] == 4 && b[0] == 1 && b[1] == 2;
}

}  // namespace

TEST_CASE("loose_swap - constexpr")
{
  // These take the ranges::swap path, which is constexpr before C++20 unlike std::swap. A class
  // without an ADL swap goes through std::swap instead, which may be explicitly specialized.
  STATIC_REQUIRE(test_adl());
  STATIC_REQUIRE(test_pointer());
  STATIC_REQUIRE(test_array());
}
