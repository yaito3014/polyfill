#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/utility.hpp>

#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

template<class T, T... Is>
pf::integer_sequence<T, Is...> to_polyfill(std::integer_sequence<T, Is...>);

template<class T, T N>
using same_as_std = std::is_same<pf::make_integer_sequence<T, N>, decltype(to_polyfill(std::make_integer_sequence<T, N>{}))>;

}  // namespace

TEST_CASE("make_integer_sequence agrees with std::make_integer_sequence")
{
  STATIC_REQUIRE(same_as_std<int, 0>::value);
  STATIC_REQUIRE(same_as_std<int, 1>::value);
  STATIC_REQUIRE(same_as_std<int, 2>::value);
  STATIC_REQUIRE(same_as_std<int, 31>::value);
  STATIC_REQUIRE(same_as_std<int, 2001>::value);
  STATIC_REQUIRE(same_as_std<unsigned char, 255>::value);
  STATIC_REQUIRE(same_as_std<std::size_t, 2000>::value);
}
