#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/invocable_traits.hpp>

#include <type_traits>

namespace ext = yk::polyfill::extension;

namespace {

struct S {};

}  // namespace

// noexcept as part of a function type: only representable from C++17.
TEST_CASE("invocable_traits noexcept")
{
  using T = ext::invocable_traits<int (*)(double) noexcept>;
  STATIC_REQUIRE(T::is_noexcept);
  STATIC_REQUIRE(std::is_same_v<T::function_type, int(double) noexcept>);

  using R = ext::invocable_traits<void (S::*)() && noexcept>;
  STATIC_REQUIRE(R::is_rvalue_reference);
  STATIC_REQUIRE(R::is_noexcept);
  STATIC_REQUIRE_FALSE(R::is_const);
  STATIC_REQUIRE(std::is_same_v<R::function_type, void() noexcept>);

  using C = ext::invocable_traits<int (S::*)(char) const noexcept>;
  STATIC_REQUIRE(C::is_const);
  STATIC_REQUIRE(C::is_noexcept);
  STATIC_REQUIRE(std::is_same_v<C::function_type, int(char) noexcept>);

  using V = ext::invocable_traits<int (S::*)() const volatile&& noexcept>;
  STATIC_REQUIRE(V::is_const);
  STATIC_REQUIRE(V::is_volatile);
  STATIC_REQUIRE(V::is_rvalue_reference);
  STATIC_REQUIRE(V::is_noexcept);
  STATIC_REQUIRE(std::is_same_v<V::function_type, int() noexcept>);

  using VA = ext::invocable_traits<int (*)(char const*, ...) noexcept>;
  STATIC_REQUIRE(VA::is_variadic);
  STATIC_REQUIRE(VA::is_noexcept);
  STATIC_REQUIRE(std::is_same_v<VA::function_type, int(char const*, ...) noexcept>);
}
