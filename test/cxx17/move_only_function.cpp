#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

int doubles_noexcept(int x) noexcept { return 2 * x; }

struct NoexceptOnlyForLvalue {
  int operator()() & noexcept { return 1; }
  int operator()() && { return 2; }
};

struct NoexceptConversion {
  operator int() const noexcept { return 42; }
};

struct ThrowingConversion {
  operator int() const { return 42; }
};

}  // namespace

TEST_CASE("move_only_function: noexcept signatures")
{
  STATIC_REQUIRE(std::is_constructible_v<pf::move_only_function<int(int) noexcept>, int (*)(int) noexcept>);
  STATIC_REQUIRE_FALSE(std::is_constructible_v<pf::move_only_function<int(int) noexcept>, int (*)(int)>);
  STATIC_REQUIRE(std::is_constructible_v<pf::move_only_function<int(int)>, int (*)(int) noexcept>);

  // the conversion to R must not throw either
  STATIC_REQUIRE(std::is_constructible_v<pf::move_only_function<int() noexcept>, NoexceptConversion (*)() noexcept>);
  STATIC_REQUIRE_FALSE(std::is_constructible_v<pf::move_only_function<int() noexcept>, ThrowingConversion (*)() noexcept>);

  // with no ref-qualifier, the call must be noexcept for both the rvalue and the lvalue target
  STATIC_REQUIRE(std::is_constructible_v<pf::move_only_function<int() & noexcept>, NoexceptOnlyForLvalue>);
  STATIC_REQUIRE_FALSE(std::is_constructible_v<pf::move_only_function<int() noexcept>, NoexceptOnlyForLvalue>);

  STATIC_REQUIRE(std::is_nothrow_invocable_v<pf::move_only_function<void() noexcept>&>);
  STATIC_REQUIRE_FALSE(std::is_nothrow_invocable_v<pf::move_only_function<void()>&>);
  STATIC_REQUIRE(std::is_nothrow_invocable_v<pf::move_only_function<void() const noexcept> const&>);
  STATIC_REQUIRE(std::is_nothrow_invocable_v<pf::move_only_function<void() && noexcept>&&>);
  STATIC_REQUIRE(std::is_nothrow_invocable_v<pf::move_only_function<void() const & noexcept> const&>);

  pf::move_only_function<int(int) noexcept> f = doubles_noexcept;
  REQUIRE(f);
  CHECK(f(21) == 42);

  pf::move_only_function<int() const && noexcept> g = []() noexcept { return 42; };
  CHECK(std::move(g)() == 42);
}

TEST_CASE("move_only_function: noexcept signatures with in_place_type, assignment and swap")
{
  STATIC_REQUIRE_FALSE(std::is_constructible_v<pf::move_only_function<int() noexcept>, pf::in_place_type_t<NoexceptOnlyForLvalue>>);
  STATIC_REQUIRE(std::is_constructible_v<pf::move_only_function<int() & noexcept>, pf::in_place_type_t<NoexceptOnlyForLvalue>>);
  STATIC_REQUIRE_FALSE(std::is_assignable_v<pf::move_only_function<int(int) noexcept>&, int (*)(int)>);
  STATIC_REQUIRE(std::is_assignable_v<pf::move_only_function<int(int) noexcept>&, int (*)(int) noexcept>);

  pf::move_only_function<int() const noexcept> f(pf::in_place_type<NoexceptConversion (*)() noexcept>, []() noexcept { return NoexceptConversion{}; });
  pf::move_only_function<int() const noexcept> g;
  CHECK(g == nullptr);
  f.swap(g);
  CHECK(f == nullptr);
  CHECK(g() == 42);
  f = std::move(g);
  CHECK(f() == 42);
  f = []() noexcept { return 1; };
  CHECK(f() == 1);
  f = nullptr;
  CHECK_FALSE(f);
}
