#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/invocable_traits.hpp>

#include <tuple>
#include <type_traits>

namespace ext = yk::polyfill::extension;

namespace {

struct S {};

}  // namespace

TEST_CASE("invocable_traits function pointer")
{
  using T = ext::invocable_traits<int (*)(double, char)>;
  STATIC_REQUIRE(std::is_same<T::return_type, int>::value);
  STATIC_REQUIRE(T::arity == 2);
  STATIC_REQUIRE_FALSE(T::is_noexcept);
  STATIC_REQUIRE_FALSE(T::is_variadic);
  STATIC_REQUIRE(std::is_same<T::apply_args<std::tuple>, std::tuple<double, char>>::value);
  STATIC_REQUIRE(std::is_same<T::function_type, int(double, char)>::value);

  // C-style variadic: the ellipsis is not counted in arity but is reflected by is_variadic.
  using V = ext::invocable_traits<int (*)(char const*, ...)>;
  STATIC_REQUIRE(V::arity == 1);
  STATIC_REQUIRE(V::is_variadic);
  STATIC_REQUIRE(std::is_same<V::function_type, int(char const*, ...)>::value);
}

TEST_CASE("invocable_traits member function pointer")
{
  using T = ext::invocable_traits<int (S::*)(char) const>;
  STATIC_REQUIRE(std::is_same<T::return_type, int>::value);
  STATIC_REQUIRE(std::is_same<T::class_type, S>::value);
  STATIC_REQUIRE(T::is_const);
  STATIC_REQUIRE_FALSE(T::is_lvalue_reference);
  STATIC_REQUIRE(std::is_same<T::function_type, int(char)>::value);

  using R = ext::invocable_traits<void (S::*)() &&>;
  STATIC_REQUIRE(R::is_rvalue_reference);
  STATIC_REQUIRE_FALSE(R::is_const);
  STATIC_REQUIRE(std::is_same<R::function_type, void()>::value);

  using V = ext::invocable_traits<int (S::*)(char) const volatile&>;
  STATIC_REQUIRE(V::is_const);
  STATIC_REQUIRE(V::is_volatile);
  STATIC_REQUIRE(V::is_lvalue_reference);
  STATIC_REQUIRE(std::is_same<V::function_type, int(char)>::value);

  using VA = ext::invocable_traits<int (S::*)(char const*, ...) const>;
  STATIC_REQUIRE(VA::is_const);
  STATIC_REQUIRE(VA::is_variadic);
  STATIC_REQUIRE(VA::arity == 1);
  STATIC_REQUIRE(std::is_same<VA::function_type, int(char const*, ...)>::value);
}

TEST_CASE("invocable_traits member object pointer")
{
  using T = ext::invocable_traits<int S::*>;
  STATIC_REQUIRE(std::is_same<T::class_type, S>::value);
  STATIC_REQUIRE(std::is_same<T::member_type, int>::value);
}
