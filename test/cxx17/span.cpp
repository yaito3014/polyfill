#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/span.hpp>
#include <yk/polyfill/type_traits.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

namespace pf = yk::polyfill;

TEST_CASE("span - deduction guides")
{
  int arr[] = {1, 2, 3};
  std::array<int, 3> sarr = {1, 2, 3};
  std::array<int, 3> const& csarr = sarr;
  std::vector<int> v = {1, 2};
  std::vector<int> const& cv = v;
  std::string str = "ab";

  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr)), pf::span<int, 3>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(sarr)), pf::span<int, 3>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(csarr)), pf::span<int const, 3>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(v)), pf::span<int>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(cv)), pf::span<int const>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(str)), pf::span<char>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr, 2)), pf::span<int>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr, arr + 2)), pf::span<int>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(static_cast<int const*>(arr), 2)), pf::span<int const>>);

  pf::span s(arr);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(s.begin(), s.end())), pf::span<int>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(s)), pf::span<int, 3>>);
}

TEST_CASE("span - deduction of a static extent from an integral-constant-like count")
{
  int arr[] = {1, 2, 3};
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr, std::integral_constant<std::size_t, 2>{})), pf::span<int, 2>>);
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr, pf::integral_constant<int, 2>{})), pf::span<int, 2>>);
  // bool is excluded from integral-constant-like.
  STATIC_REQUIRE(std::is_same_v<decltype(pf::span(arr, std::true_type{})), pf::span<int>>);

  pf::span s(arr, std::integral_constant<std::size_t, 2>{});
  CHECK(s.data() == arr);
  CHECK(s.size() == 2);
}

namespace {

constexpr bool test_reverse_iterators()
{
  int arr[] = {1, 2, 3};
  pf::span<int> s = arr;
  return *s.rbegin() == 3 && s.rend() - s.rbegin() == 3 && *s.crbegin() == 3 && s.crend() - s.crbegin() == 3;
}

constexpr bool test_std_array()
{
  std::array<int, 3> arr = {1, 2, 3};
  std::array<int, 3> const& carr = arr;
  pf::span<int, 3> s = arr;
  pf::span<int const> c = carr;
  return s.data() == arr.data() && c.size() == 3;
}

}  // namespace

TEST_CASE("span - constexpr reverse iterators and std::array construction")
{
  STATIC_REQUIRE(test_reverse_iterators());
  STATIC_REQUIRE(test_std_array());
}

TEST_CASE("span - as_bytes and as_writable_bytes")
{
  int arr[] = {1, 2, 3};
  pf::span<int, 3> f = arr;
  pf::span<int> d = arr;

  auto cb = pf::as_bytes(f);
  STATIC_REQUIRE(std::is_same_v<decltype(cb), pf::span<std::byte const, 3 * sizeof(int)>>);
  CHECK(static_cast<void const*>(cb.data()) == static_cast<void const*>(arr));
  CHECK(cb.size() == sizeof(arr));

  auto wb = pf::as_writable_bytes(d);
  STATIC_REQUIRE(std::is_same_v<decltype(wb), pf::span<std::byte>>);
  CHECK(static_cast<void*>(wb.data()) == static_cast<void*>(arr));
  CHECK(wb.size() == sizeof(arr));

  STATIC_REQUIRE(noexcept(pf::as_bytes(f)));
  STATIC_REQUIRE(noexcept(pf::as_writable_bytes(f)));
}

namespace {

template<class S, class = void>
constexpr bool can_as_writable_bytes = false;

template<class S>
constexpr bool can_as_writable_bytes<S, std::void_t<decltype(pf::as_writable_bytes(std::declval<S>()))>> = true;

template<class S, class = void>
constexpr bool can_as_bytes = false;

template<class S>
constexpr bool can_as_bytes<S, std::void_t<decltype(pf::as_bytes(std::declval<S>()))>> = true;

}  // namespace

TEST_CASE("span - as_bytes constraints")
{
  STATIC_REQUIRE(can_as_bytes<pf::span<int const>>);
  STATIC_REQUIRE(!can_as_bytes<pf::span<int volatile>>);
  STATIC_REQUIRE(can_as_writable_bytes<pf::span<int>>);
  STATIC_REQUIRE(!can_as_writable_bytes<pf::span<int const>>);
  STATIC_REQUIRE(!can_as_writable_bytes<pf::span<int volatile>>);
}
