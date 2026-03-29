#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/memory.hpp>

#include <functional>
#include <memory>

#include <cstdarg>

int normal_function(int a, int b) { return a + b; }
int nothrow_function(int a, int b) noexcept { return a + b; }

struct S {
  int value;
  explicit S(int val) noexcept : value(val) {}

  int& mutating_member_function(int arg) { return value = arg; }
  int const_member_function(int arg) const { return value + arg; }
  int& mutating_nothrow_member_function(int arg) noexcept { return value = arg; }
  int const_nothrow_member_function(int arg) const noexcept { return value + arg; }

  int volatile& mutating_volatile_member_function(int arg) volatile { return value = arg; }
  int const_volatile_member_function(int arg) const volatile { return value + arg; }
  int volatile& mutating_volatile_nothrow_member_function(int arg) volatile noexcept { return value = arg; }
  int const_volatile_nothrow_member_function(int arg) const volatile noexcept { return value + arg; }

  int& mutating_lvalue_ref_member_function(int arg) & { return value = arg; }
  int const_lvalue_ref_member_function(int arg) const& { return value + arg; }
  int& mutating_lvalue_ref_nothrow_member_function(int arg) & noexcept { return value = arg; }
  int const_lvalue_ref_nothrow_member_function(int arg) const& noexcept { return value + arg; }

  int volatile& mutating_volatile_lvalue_ref_member_function(int arg) volatile& { return value = arg; }
  int const_volatile_lvalue_ref_member_function(int arg) const volatile& { return value + arg; }
  int volatile& mutating_volatile_lvalue_ref_nothrow_member_function(int arg) volatile& noexcept { return value = arg; }
  int const_volatile_lvalue_ref_nothrow_member_function(int arg) const volatile& noexcept { return value + arg; }

  int mutating_rvalue_ref_member_function(int arg) && { return value = arg; }
  int const_rvalue_ref_member_function(int arg) const&& { return value + arg; }
  int mutating_rvalue_ref_nothrow_member_function(int arg) && noexcept { return value = arg; }
  int const_rvalue_ref_nothrow_member_function(int arg) const&& noexcept { return value + arg; }

  int mutating_volatile_rvalue_ref_member_function(int arg) volatile&& { return value = arg; }
  int const_volatile_rvalue_ref_member_function(int arg) const volatile&& { return value + arg; }
  int mutating_volatile_rvalue_ref_nothrow_member_function(int arg) volatile&& noexcept { return value = arg; }
  int const_volatile_rvalue_ref_nothrow_member_function(int arg) const volatile&& noexcept { return value + arg; }

  // C-style variadic member functions
  int variadic_member_function(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_variadic_member_function(int count, ...) const {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int volatile_variadic_member_function(int count, ...) volatile {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_volatile_variadic_member_function(int count, ...) const volatile {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int lvalue_ref_variadic_member_function(int count, ...) & {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_lvalue_ref_variadic_member_function(int count, ...) const& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int volatile_lvalue_ref_variadic_member_function(int count, ...) volatile& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_volatile_lvalue_ref_variadic_member_function(int count, ...) const volatile& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int rvalue_ref_variadic_member_function(int count, ...) && {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_rvalue_ref_variadic_member_function(int count, ...) const&& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int volatile_rvalue_ref_variadic_member_function(int count, ...) volatile&& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
  int const_volatile_rvalue_ref_variadic_member_function(int count, ...) const volatile&& {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(args, int);
    va_end(args);
    return value + sum;
  }
};

struct Derived : S {
  explicit Derived(int val) noexcept : S(val) {}
};

struct ThrowingConvertible {
  ThrowingConvertible(int) noexcept(false) {}
};

struct NothrowConvertible {
  NothrowConvertible(int) noexcept {}
};

struct InvocationIsThrowing {
  void mutation_member_function(ThrowingConvertible) {}
  void const_member_function(ThrowingConvertible) const {}
  void mutation_nothrow_member_function(ThrowingConvertible) noexcept {}
  void const_nothrow_member_function(ThrowingConvertible) const noexcept {}
};

namespace pf = yk::polyfill;

TEST_CASE("invoke")
{
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

  // std::function
  {
    std::function<int(int, int)> fn = normal_function;
    CHECK(pf::invoke(fn, 33, 4) == 37);
  }

  // member function pointer + object (lvalue)
  {
    S s{12};
    CHECK(pf::invoke(&S::mutating_member_function, s, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, s, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, s, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_member_function, s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, s, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, s, 34) == 34);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, s, 78) == 78);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, s, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, s, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, s, 90) == 168);
  }

  // member function pointer + object (xvalue)
  {
    CHECK(pf::invoke(&S::mutating_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_nothrow_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_rvalue_ref_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_rvalue_ref_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_rvalue_ref_nothrow_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_rvalue_ref_nothrow_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_rvalue_ref_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_rvalue_ref_member_function, S{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_rvalue_ref_nothrow_member_function, S{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_rvalue_ref_nothrow_member_function, S{33}, 4) == 37);
  }

  // member function pointer + reference_wrapper (lvalue)
  {
    S s{33};
    auto ref = std::ref(s);
    CHECK(pf::invoke(&S::mutating_member_function, ref, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, ref, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, ref, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, ref, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, ref, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_member_function, ref, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, ref, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, ref, 90) == 168);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, ref, 34) == 34);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, ref, 56) == 90);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, ref, 78) == 78);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, ref, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, ref, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, ref, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, ref, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, ref, 90) == 168);
  }

  // member function pointer + reference_wrapper (xvalue)
  {
    S s{33};
    CHECK(pf::invoke(&S::mutating_member_function, std::ref(s), 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, std::ref(s), 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, std::ref(s), 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, std::ref(s), 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, std::ref(s), 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_member_function, std::ref(s), 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, std::ref(s), 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, std::ref(s), 90) == 168);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, std::ref(s), 34) == 34);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, std::ref(s), 56) == 90);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, std::ref(s), 78) == 78);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, std::ref(s), 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, std::ref(s), 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, std::ref(s), 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, std::ref(s), 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, std::ref(s), 90) == 168);
  }

  // member function pointer + pointer
  {
    S s{33};
    CHECK(pf::invoke(&S::mutating_member_function, &s, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, &s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, &s, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, &s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, &s, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_member_function, &s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, &s, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, &s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, &s, 34) == 34);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, &s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, &s, 78) == 78);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, &s, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, &s, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, &s, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, &s, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, &s, 90) == 168);
  }

  // member function pointer + pointer-ish (lvalue)
  {
    auto ptr = pf::make_unique<S>(33);
    CHECK(pf::invoke(&S::mutating_member_function, ptr, 34) == 34);
    CHECK(pf::invoke(&S::const_member_function, ptr, 56) == 90);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, ptr, 78) == 78);
    CHECK(pf::invoke(&S::const_nothrow_member_function, ptr, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, ptr, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_member_function, ptr, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, ptr, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, ptr, 90) == 168);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, ptr, 34) == 34);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, ptr, 56) == 90);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, ptr, 78) == 78);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, ptr, 90) == 168);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, ptr, 34) == 34);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, ptr, 56) == 90);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, ptr, 78) == 78);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, ptr, 90) == 168);
  }

  // member function pointer + pointer-ish (xvalue)
  {
    CHECK(pf::invoke(&S::mutating_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_nothrow_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, pf::make_unique<S>(33), 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, pf::make_unique<S>(33), 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, pf::make_unique<S>(33), 4) == 37);
  }

  // member function pointer + derived class
  {
    Derived d{10};
    CHECK(pf::invoke(&S::const_member_function, d, 5) == 15);
    CHECK(pf::invoke(&S::mutating_member_function, d, 20) == 20);
    CHECK(pf::invoke(&S::const_nothrow_member_function, d, 3) == 23);
    CHECK(pf::invoke(&S::mutating_nothrow_member_function, d, 40) == 40);
    CHECK(pf::invoke(&S::const_volatile_member_function, d, 5) == 45);
    CHECK(pf::invoke(&S::mutating_volatile_member_function, d, 20) == 20);
    CHECK(pf::invoke(&S::const_volatile_nothrow_member_function, d, 3) == 23);
    CHECK(pf::invoke(&S::mutating_volatile_nothrow_member_function, d, 40) == 40);
    CHECK(pf::invoke(&S::const_lvalue_ref_member_function, d, 5) == 45);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_member_function, d, 20) == 20);
    CHECK(pf::invoke(&S::const_lvalue_ref_nothrow_member_function, d, 3) == 23);
    CHECK(pf::invoke(&S::mutating_lvalue_ref_nothrow_member_function, d, 40) == 40);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_member_function, d, 5) == 45);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_member_function, d, 20) == 20);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_nothrow_member_function, d, 3) == 23);
    CHECK(pf::invoke(&S::mutating_volatile_lvalue_ref_nothrow_member_function, d, 40) == 40);
    CHECK(pf::invoke(&S::mutating_rvalue_ref_member_function, Derived{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_rvalue_ref_member_function, Derived{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_rvalue_ref_nothrow_member_function, Derived{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_rvalue_ref_nothrow_member_function, Derived{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_rvalue_ref_member_function, Derived{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_rvalue_ref_member_function, Derived{33}, 4) == 37);
    CHECK(pf::invoke(&S::mutating_volatile_rvalue_ref_nothrow_member_function, Derived{33}, 4) == 4);
    CHECK(pf::invoke(&S::const_volatile_rvalue_ref_nothrow_member_function, Derived{33}, 4) == 37);
  }

  // C-style variadic member function + object (lvalue)
  {
    S s{10};
    CHECK(pf::invoke(&S::variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::lvalue_ref_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_lvalue_ref_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_lvalue_ref_variadic_member_function, s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_variadic_member_function, s, 2, 3, 4) == 17);
  }

  // C-style variadic member function + object (xvalue)
  {
    CHECK(pf::invoke(&S::variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::rvalue_ref_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_rvalue_ref_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_rvalue_ref_variadic_member_function, S{10}, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_rvalue_ref_variadic_member_function, S{10}, 2, 3, 4) == 17);
  }

  // C-style variadic member function + pointer
  {
    S s{10};
    CHECK(pf::invoke(&S::variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::lvalue_ref_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_lvalue_ref_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_lvalue_ref_variadic_member_function, &s, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_variadic_member_function, &s, 2, 3, 4) == 17);
  }

  // C-style variadic member function + reference_wrapper
  {
    S s{10};
    CHECK(pf::invoke(&S::variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::lvalue_ref_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_lvalue_ref_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_lvalue_ref_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_variadic_member_function, std::ref(s), 2, 3, 4) == 17);
  }

  // C-style variadic member function + pointer-ish
  {
    auto ptr = pf::make_unique<S>(10);
    CHECK(pf::invoke(&S::variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::lvalue_ref_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_lvalue_ref_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::volatile_lvalue_ref_variadic_member_function, ptr, 2, 3, 4) == 17);
    CHECK(pf::invoke(&S::const_volatile_lvalue_ref_variadic_member_function, ptr, 2, 3, 4) == 17);
  }

  // member data pointer + object (lvalue)
  {
    S s{42};
    CHECK(pf::invoke(&S::value, s) == 42);
  }

  // member data pointer + object (xvalue)
  {
    CHECK(pf::invoke(&S::value, S{42}) == 42);
  }

  // member data pointer + reference_wrapper
  {
    S s{42};
    CHECK(pf::invoke(&S::value, std::ref(s)) == 42);
    CHECK(pf::invoke(&S::value, std::cref(s)) == 42);
  }

  // member data pointer + pointer
  {
    S s{42};
    CHECK(pf::invoke(&S::value, &s) == 42);
  }

  // member data pointer + pointer-ish
  {
    auto ptr = pf::make_unique<S>(42);
    CHECK(pf::invoke(&S::value, ptr) == 42);
  }

  // member data pointer + derived class
  {
    Derived d{42};
    CHECK(pf::invoke(&S::value, d) == 42);
  }
}

TEST_CASE("invoke_r")
{
  // invoke_r with implicit conversion
  CHECK(pf::invoke_r<double>(&normal_function, 33, 4) == 37.0);

  // invoke_r<void> discards return value
  {
    int side_effect = 0;
    pf::invoke_r<void>([&] { side_effect = 1; });
    CHECK(side_effect == 1);
  }

  // invoke_r with member function
  {
    S s{10};
    CHECK(pf::invoke_r<long>(&S::const_member_function, s, 5) == 15L);
  }

  // invoke_r with member data pointer
  {
    S s{42};
    CHECK(pf::invoke_r<long>(&S::value, s) == 42L);
  }

  // invoke_r with lambda
  {
    auto fn = [](int a, int b) -> int { return a + b; };
    CHECK(pf::invoke_r<double>(fn, 33, 4) == 37.0);
  }
}

TEST_CASE("is_invocable")
{
  // normal function
  STATIC_REQUIRE(pf::is_invocable<decltype(&normal_function), int, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&normal_function), int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&normal_function)>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&normal_function), int, int, int>::value == false);

  // member data pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::value), S>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::value), S&>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::value), S*>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::value)>::value == false);

  // derived class with base member pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::value), Derived>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), Derived, int>::value == true);

  // member function pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S const&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S const&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S const&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S const&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_member_function), int, int>::value == false);

  // lvalue ref qualified member function pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S const&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_lvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S const&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), S*, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_lvalue_ref_member_function), int, int>::value == false);

  // rvalue ref qualified member function pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S const&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), S*, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), S*, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_rvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S const&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), S*, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_member_function), int, int>::value == false);

  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S const&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S&&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S const&&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), S*, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::mutating_volatile_rvalue_ref_member_function), int, int>::value == false);

  // C-style variadic member function pointer
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::variadic_member_function), S&, int, int, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::variadic_member_function), S*, int, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_variadic_member_function), S const&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_variadic_member_function), S*, int, int, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::volatile_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::lvalue_ref_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::lvalue_ref_variadic_member_function), S, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_lvalue_ref_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::volatile_lvalue_ref_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_lvalue_ref_variadic_member_function), S&, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::rvalue_ref_variadic_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::rvalue_ref_variadic_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_rvalue_ref_variadic_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::volatile_rvalue_ref_variadic_member_function), S, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable<decltype(&S::const_volatile_rvalue_ref_variadic_member_function), S, int>::value == true);

  // not invocable
  STATIC_REQUIRE(pf::is_invocable<int>::value == false);
  STATIC_REQUIRE(pf::is_invocable<int, int>::value == false);
}

TEST_CASE("is_nothrow_invocable")
{
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&normal_function), int, int>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&S::const_member_function), S&, int>::value == false);

  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&InvocationIsThrowing::const_member_function), S&, int>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&InvocationIsThrowing::const_nothrow_member_function), S&, int>::value == false);

#if __cpp_noexcept_function_type >= 201510
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&nothrow_function), int, int>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&S::const_nothrow_member_function), S&, int>::value == true);

  // member function pointer itself is marked as noexcept, but invocation is throwing
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&InvocationIsThrowing::const_nothrow_member_function), S&, int>::value == false);
#endif

  // member data pointer is always noexcept
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&S::value), S&>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_invocable<decltype(&S::value), S*>::value == true);
}

TEST_CASE("is_invocable_r")
{
  // exact return type
  STATIC_REQUIRE(pf::is_invocable_r<int, decltype(&normal_function), int, int>::value == true);
  // convertible return type
  STATIC_REQUIRE(pf::is_invocable_r<double, decltype(&normal_function), int, int>::value == true);
  STATIC_REQUIRE(pf::is_invocable_r<long, decltype(&normal_function), int, int>::value == true);
  // void discards
  STATIC_REQUIRE(pf::is_invocable_r<void, decltype(&normal_function), int, int>::value == true);

  // member data pointer
  STATIC_REQUIRE(pf::is_invocable_r<int, decltype(&S::value), S&>::value == true);
  STATIC_REQUIRE(pf::is_invocable_r<long, decltype(&S::value), S&>::value == true);

  // not invocable
  STATIC_REQUIRE(pf::is_invocable_r<int, int>::value == false);
  STATIC_REQUIRE(pf::is_invocable_r<int, int, int>::value == false);

  // wrong arity
  STATIC_REQUIRE(pf::is_invocable_r<int, decltype(&normal_function), int>::value == false);
  STATIC_REQUIRE(pf::is_invocable_r<int, decltype(&normal_function), int, int, int>::value == false);

  // non-convertible return type
  STATIC_REQUIRE(pf::is_invocable_r<S, decltype(&normal_function), int, int>::value == false);
}

TEST_CASE("is_nothrow_invocable_r")
{
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<int, decltype(&normal_function), int, int>::value == false);

  // member data pointer is noexcept
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<int, decltype(&S::value), S&>::value == true);

  // not invocable
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<int, int>::value == false);
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<int, int, int>::value == false);

  // non-convertible return type
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<S, decltype(&nothrow_function), int, int>::value == false);

  // nothrow invocable but throwing conversion
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<ThrowingConvertible, decltype(&nothrow_function), int, int>::value == false);

#if __cpp_noexcept_function_type >= 201510
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<int, decltype(&nothrow_function), int, int>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<double, decltype(&nothrow_function), int, int>::value == true);

  // nothrow invocable and nothrow conversion
  STATIC_REQUIRE(pf::is_nothrow_invocable_r<NothrowConvertible, decltype(&nothrow_function), int, int>::value == true);
#endif
}

TEST_CASE("invoke_result")
{
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&normal_function), int, int>::type, int>::value == true);
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&S::const_member_function), S, int>::type, int>::value == true);
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&S::mutating_member_function), S&, int>::type, int&>::value == true);
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&S::value), S&>::type, int&>::value == true);
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&S::value), S const&>::type, int const&>::value == true);
  STATIC_REQUIRE(std::is_same<pf::invoke_result<decltype(&S::value), S&&>::type, int&&>::value == true);
}
