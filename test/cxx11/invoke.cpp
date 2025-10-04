#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/cxx11/invoke.hpp>
#include <yk/polyfill/cxx11/make_unique.hpp>

#include <functional>
#include <memory>

int normal_function(int a, int b) { return a + b; }

struct S {
  int value;
  explicit S(int val) noexcept : value(val) {}

  int& mutating_member_function(int arg) { return value = arg; }
  int const_member_function(int arg) const { return value + arg; }
  int& mutating_nothrow_member_function(int arg) noexcept { return value = arg; }
  int const_nothrow_member_function(int arg) const noexcept { return value + arg; }
};

namespace pf = yk::polyfill;

TEST_CASE("invoke")
{
  // TODO: more tests on invoke related type traits

  CHECK(pf::invoke(&normal_function, 33, 4) == 37);

  {
    auto non_capturing_function_object = [](int a, int b) -> int { return a + b; };
    CHECK(pf::invoke(non_capturing_function_object, 33, 4) == 37);
  }

  {
    int x = 12;
    auto capturing_function_object = [&](int arg) -> int { return arg * x; };
    CHECK(pf::invoke(capturing_function_object, 34) == 408);
  }

  // object (lvalue)
  {
    S s{12};
    CHECK(pf::invoke(&S::mutating_member_function, s, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, s, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, s, 90) == 168);
  }

  // object (xvalue)
  {
    CHECK(pf::invoke(&S::mutating_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_nothrow_member_function, S{33}, 4) == 37);
  }

  // reference_wrapper (lvalue)
  {
    S s{33};
    auto ref = std::ref(s);
    CHECK(pf::invoke(&S::mutating_member_function, ref, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, ref, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, ref, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, ref, 90) == 168);
  }

  // reference_wrapper (xvalue)
  {
    S s{33};
    CHECK(pf::invoke(&S::mutating_member_function, std::ref(s), 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, std::ref(s), 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, std::ref(s), 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, std::ref(s), 90) == 168);
  }

  // pointer
  {
    S s{33};
    CHECK(pf::invoke(&S::mutating_member_function, &s, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, &s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, &s, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, &s, 90) == 168);
  }

  // pointer-ish (lvalue)
  {
    auto ptr = pf::make_unique<S>(33);
    CHECK(pf::invoke(&S::mutating_member_function, ptr, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, ptr, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, ptr, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, ptr, 90) == 168);
  }

  // pointer-ish (xvalue)
  {
    CHECK(pf::invoke(&S::mutating_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_nothrow_member_function, pf::make_unique<S>(33), 4) == 37);
  }
}
