#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

struct true_type {
  static constexpr bool value = true;
};

struct false_type {
  static constexpr bool value = false;
};

struct marked_false : false_type {
  using marker = int;
};

template<class T>
struct always_false : false_type {};

template<class T>
struct always_fail {
  static_assert(always_false<T>::value, "fail");
};

namespace pf = yk::polyfill;

TEST_CASE("conjunction")
{
  STATIC_REQUIRE(pf::conjunction<>::value == true);

  STATIC_REQUIRE(pf::conjunction<true_type>::value == true);
  STATIC_REQUIRE(pf::conjunction<false_type>::value == false);

  STATIC_REQUIRE(pf::conjunction<true_type, true_type>::value == true);
  STATIC_REQUIRE(pf::conjunction<true_type, false_type>::value == false);
  STATIC_REQUIRE(pf::conjunction<false_type, true_type>::value == false);
  STATIC_REQUIRE(pf::conjunction<false_type, false_type>::value == false);

  // short-circuit
  STATIC_REQUIRE(pf::conjunction<false_type, always_fail<int>>::value == false);
  STATIC_REQUIRE(std::is_base_of<marked_false, pf::conjunction<marked_false>>::value);
  STATIC_REQUIRE(std::is_base_of<marked_false, pf::conjunction<true_type, marked_false, always_fail<int>>>::value);
}
