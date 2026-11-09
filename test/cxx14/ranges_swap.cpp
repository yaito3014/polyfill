#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/concepts.hpp>

namespace pf = yk::polyfill;

namespace {

struct aggregate {
  int value;
};

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

constexpr bool test_scalar()
{
  int a = 1, b = 2;
  pf::ranges::swap(a, b);
  return a == 2 && b == 1;
}

constexpr bool test_aggregate()
{
  aggregate a{10}, b{20};
  pf::ranges::swap(a, b);
  return a.value == 20 && b.value == 10;
}

constexpr bool test_adl()
{
  constexpr_adl_swap a{1, false}, b{2, false};
  pf::ranges::swap(a, b);
  return a.value == 2 && b.value == 1 && a.via_adl;
}

constexpr bool test_array()
{
  int a[2] = {1, 2}, b[2] = {3, 4};
  pf::ranges::swap(a, b);
  return a[0] == 3 && a[1] == 4 && b[0] == 1 && b[1] == 2;
}

constexpr bool test_multidimensional_array()
{
  int a[2][2] = {{1, 2}, {3, 4}}, b[2][2] = {{5, 6}, {7, 8}};
  pf::ranges::swap(a, b);
  return a[0][0] == 5 && a[1][1] == 8 && b[0][0] == 1 && b[1][1] == 4;
}

}  // namespace

TEST_CASE("ranges::swap - constexpr")
{
  STATIC_REQUIRE(test_scalar());
  STATIC_REQUIRE(test_aggregate());
  STATIC_REQUIRE(test_adl());
  STATIC_REQUIRE(test_array());
  STATIC_REQUIRE(test_multidimensional_array());
}
