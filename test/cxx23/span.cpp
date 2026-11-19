#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/span.hpp>

#include <iterator>
#include <ranges>
#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("span - const iterators")
{
  using S = pf::span<int>;
#if __cpp_lib_ranges_as_const >= 202207L
  STATIC_REQUIRE(std::is_same_v<S::const_iterator, std::const_iterator<S::iterator>>);
  STATIC_REQUIRE(std::is_same_v<S::const_reverse_iterator, std::const_iterator<S::reverse_iterator>>);
#endif

  int arr[] = {1, 2, 3};
  S s = arr;
  CHECK(std::ranges::cbegin(s) == s.cbegin());
  STATIC_REQUIRE(std::is_same_v<decltype(*s.cbegin()), int const&>);
  STATIC_REQUIRE(std::is_same_v<decltype(*s.crbegin()), int const&>);
}
