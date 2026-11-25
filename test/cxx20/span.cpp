#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/span.hpp>

#include <array>
#include <iterator>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace pf = yk::polyfill;

namespace {

// A contiguous range whose only means of access is begin() and end().
struct IteratorOnlyRange {
  int elems[2];
  int* begin() { return elems; }
  int* end() { return elems + 2; }
};

// A contiguous range that is not sized.
struct UnsizedRange {
  int elems[2];
  int* begin() { return elems; }
  std::unreachable_sentinel_t end() { return {}; }
};

}  // namespace

TEST_CASE("span - ranges integration")
{
  STATIC_REQUIRE(std::contiguous_iterator<pf::span<int>::iterator>);
  STATIC_REQUIRE(std::contiguous_iterator<pf::span<int const>::iterator>);
  STATIC_REQUIRE(std::ranges::contiguous_range<pf::span<int>>);
  STATIC_REQUIRE(std::ranges::sized_range<pf::span<int>>);
  STATIC_REQUIRE(std::ranges::view<pf::span<int>>);
  STATIC_REQUIRE(std::ranges::view<pf::span<int, 3>>);
  STATIC_REQUIRE(std::ranges::borrowed_range<pf::span<int>>);
  STATIC_REQUIRE(std::ranges::enable_borrowed_range<pf::span<int, 3>>);
  STATIC_REQUIRE(std::ranges::enable_view<pf::span<int, 3>>);

  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;
  CHECK(std::to_address(s.begin()) == arr);
  CHECK(std::ranges::data(s) == arr);
  CHECK(std::ranges::size(s) == 3);
}

TEST_CASE("span - construction from contiguous iterators")
{
  std::vector<int> v = {1, 2, 3};

  pf::span<int> s(v.begin(), 2);
  CHECK(s.data() == v.data());
  CHECK(s.size() == 2);

  pf::span<int> t(v.begin(), v.end());
  CHECK(t.size() == 3);

  pf::span<int const> c(v.cbegin() + 1, v.cend());
  CHECK(c.data() == v.data() + 1);
  CHECK(c.size() == 2);

  STATIC_REQUIRE(!std::is_constructible_v<pf::span<int>, std::vector<int>::const_iterator, std::size_t>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(v.begin(), v.end())), pf::span<int>>);
}

TEST_CASE("span - construction from ranges")
{
  IteratorOnlyRange r = {{1, 2}};
  pf::span<int> s = r;
  CHECK(s.data() == r.elems);
  CHECK(s.size() == 2);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(r)), pf::span<int>>);

  STATIC_REQUIRE(!std::is_constructible_v<pf::span<int>, UnsizedRange&>);

  // A borrowed rvalue range may be viewed through non-const elements.
  int arr[] = {1, 2, 3};
  pf::span<int> from_std_span = std::span<int>(arr);
  CHECK(from_std_span.data() == arr);
  CHECK(from_std_span.size() == 3);
  STATIC_REQUIRE(std::is_constructible_v<pf::span<int>, std::span<int>>);
  STATIC_REQUIRE(!std::is_constructible_v<pf::span<int>, std::vector<int>>);

  pf::span<char const> sv = std::string_view("abc");
  CHECK(sv.size() == 3);

  std::span<int> to_std_span = pf::span<int>(arr);
  CHECK(to_std_span.data() == arr);
  CHECK(to_std_span.size() == 3);
}

TEST_CASE("span - constexpr")
{
  constexpr auto sum = [] {
    std::array<int, 4> arr = {1, 2, 3, 4};
    pf::span<int> s = arr;
    int total = 0;
    for (int x : s.subspan(1)) total += x;
    return total;
  }();
  STATIC_REQUIRE(sum == 9);
}
