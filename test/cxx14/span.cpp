#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/span.hpp>

namespace pf = yk::polyfill;

namespace {

constexpr bool test_assignment()
{
  int a[] = {1, 2};
  int b[] = {3, 4, 5};
  pf::span<int> s = a;
  s = pf::span<int>(b);
  return s.data() == b && s.size() == 3;
}

constexpr bool test_iterator_arithmetic()
{
  int arr[] = {1, 2, 3, 4};
  pf::span<int> s = arr;
  auto it = s.begin();
  ++it;
  it++;
  --it;
  it--;
  it += 3;
  it -= 1;
  return *it == 3 && it - s.begin() == 2 && it[1] == 4 && s.end() - it == 2 && it > s.begin();
}

constexpr bool test_iterator_write()
{
  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;
  for (auto it = s.begin(); it != s.end(); ++it) *it *= 2;
  return arr[0] == 2 && arr[1] == 4 && arr[2] == 6;
}

constexpr int test_range_for()
{
  int arr[] = {1, 2, 3};
  int sum = 0;
  for (int x : pf::span<int>(arr)) sum += x;
  return sum;
}

constexpr bool test_const_iterator()
{
  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;
  auto it = s.cbegin();
  ++it;
  return *it == 2 && s.cend() - it == 2;
}

}  // namespace

TEST_CASE("span - constexpr assignment and iteration")
{
  STATIC_REQUIRE(test_assignment());
  STATIC_REQUIRE(test_iterator_arithmetic());
  STATIC_REQUIRE(test_iterator_write());
  STATIC_REQUIRE(test_range_for() == 6);
  STATIC_REQUIRE(test_const_iterator());
}
