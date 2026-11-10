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

constexpr bool test_aggregate()
{
  aggregate a{1}, b{2};
  pf::detail::loose_swap(a, b);
  return a.value == 2 && b.value == 1;
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
  // takes the ranges::swap path, which is constexpr before C++20 unlike std::swap
  STATIC_REQUIRE(test_aggregate());
  STATIC_REQUIRE(test_array());
}
