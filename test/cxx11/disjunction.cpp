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

struct marked_true : true_type {
  using marker = int;
};

struct false_type {
  static constexpr bool value = false;
};

struct marked_false : false_type {
  using marker = int;
};

// [meta.logical] only needs a value convertible to bool
struct int_two {
  static constexpr int value = 2;
};

struct int_zero {
  static constexpr int value = 0;
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
  STATIC_REQUIRE(std::is_base_of<marked_true, pf::disjunction<marked_true>>::value);
  STATIC_REQUIRE(std::is_base_of<marked_true, pf::disjunction<false_type, marked_true, always_fail<int>>>::value);

  // the last trait when none is true
  STATIC_REQUIRE(std::is_base_of<marked_false, pf::disjunction<false_type, marked_false>>::value);

  STATIC_REQUIRE(std::is_base_of<int_two, pf::disjunction<int_zero, int_two, always_fail<int>>>::value);
}
