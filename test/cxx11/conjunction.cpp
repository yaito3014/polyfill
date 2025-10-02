#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/conjunction.hpp>

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

TEST_CASE("conjunction")
{
  STATIC_CHECK(pf::conjunction<>::value == true);

  STATIC_CHECK(pf::conjunction<true_type>::value == true);
  STATIC_CHECK(pf::conjunction<false_type>::value == false);

  STATIC_CHECK(pf::conjunction<true_type, true_type>::value == true);
  STATIC_CHECK(pf::conjunction<true_type, false_type>::value == false);
  STATIC_CHECK(pf::conjunction<false_type, true_type>::value == false);
  STATIC_CHECK(pf::conjunction<false_type, false_type>::value == false);

  // short-circuit
  STATIC_CHECK(pf::conjunction<false_type, always_fail<int>>::value == false);
}
