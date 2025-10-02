#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/disjunction.hpp>

struct true_type {
  static constexpr bool value = true;
};

struct false_type {
  static constexpr bool value = false;
};

template<class T>
struct always_false : false_type {};

template<class T>
struct always_fail {
  static_assert(always_false<T>::value, "fail");
};

namespace pf = yk::polyfill;

TEST_CASE("disjunction")
{
  STATIC_REQUIRE(pf::disjunction<>::value == false);

  STATIC_REQUIRE(pf::disjunction<true_type>::value == true);
  STATIC_REQUIRE(pf::disjunction<false_type>::value == false);

  STATIC_REQUIRE(pf::disjunction<true_type, true_type>::value == true);
  STATIC_REQUIRE(pf::disjunction<true_type, false_type>::value == true);
  STATIC_REQUIRE(pf::disjunction<false_type, true_type>::value == true);
  STATIC_REQUIRE(pf::disjunction<false_type, false_type>::value == false);

  // short-circuit
  STATIC_REQUIRE(pf::disjunction<true_type, always_fail<int>>::value == true);
}
