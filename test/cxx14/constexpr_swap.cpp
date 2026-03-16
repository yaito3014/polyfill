#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/bits/swap.hpp>

namespace {

struct no_custom_swap {
  int value;
};

constexpr bool test_constexpr_swap_scalar()
{
  int a = 1, b = 2;
  yk::polyfill::detail::constexpr_swap(a, b);
  return a == 2 && b == 1;
}

constexpr bool test_constexpr_swap_aggregate()
{
  no_custom_swap a{10}, b{20};
  yk::polyfill::detail::constexpr_swap(a, b);
  return a.value == 20 && b.value == 10;
}

}  // namespace

TEST_CASE("constexpr_swap - constexpr")
{
  SECTION("scalar")
  {
    STATIC_REQUIRE(test_constexpr_swap_scalar());
  }

  SECTION("aggregate")
  {
    STATIC_REQUIRE(test_constexpr_swap_aggregate());
  }
}
