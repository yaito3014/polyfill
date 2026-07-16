#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/expected.hpp>
#include <yk/polyfill/utility.hpp>

#include <string>
#include <type_traits>

namespace pf = yk::polyfill;

namespace {

// copy/move constructible + assignable, but with a potentially-throwing move constructor.
struct ThrowMove {
  ThrowMove() = default;
  ThrowMove(ThrowMove const&) = default;
  ThrowMove(ThrowMove&&) noexcept(false) {}
  ThrowMove& operator=(ThrowMove const&) = default;
  ThrowMove& operator=(ThrowMove&&) = default;
};

}  // namespace

TEST_CASE("expected swap constraints observed through is_swappable")
{
  // std::swap is constrained on is_move_constructible && is_move_assignable only from C++17,
  // so this is the earliest standard where is_swappable observes the
  // [expected.object.swap] nothrow-move clause end to end.
  STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible<ThrowMove>::value);
  STATIC_REQUIRE_FALSE(pf::is_swappable<pf::expected<ThrowMove, ThrowMove>>::value);

  // one nothrow-move-constructible arm restores swappability
  STATIC_REQUIRE(pf::is_swappable<pf::expected<ThrowMove, int>>::value);
  STATIC_REQUIRE(pf::is_swappable<pf::expected<int, std::string>>::value);
  STATIC_REQUIRE(pf::is_swappable<pf::expected<void, int>>::value);
}

TEST_CASE("unexpected CTAD")
{
  pf::unexpected u(5);
  STATIC_REQUIRE(std::is_same<decltype(u), pf::unexpected<int>>::value);
  CHECK(u.error() == 5);

  pf::unexpected s(std::string("boom"));
  STATIC_REQUIRE(std::is_same<decltype(s), pf::unexpected<std::string>>::value);
  CHECK(s.error() == "boom");
}

TEST_CASE("expected from CTAD-deduced unexpected")
{
  pf::expected<int, std::string> e = pf::unexpected(std::string("err"));
  CHECK(!e.has_value());
  CHECK(e.error() == "err");
}

TEST_CASE("expected constexpr C++17 monadic")
{
  constexpr pf::expected<int, int> e(pf::in_place, 4);

  constexpr auto a = e.and_then([](int x) { return pf::expected<int, int>(pf::in_place, x * 2); });
  STATIC_REQUIRE(a.has_value());
  STATIC_REQUIRE(*a == 8);

  constexpr pf::expected<int, int> u(pf::unexpect, 9);
  constexpr auto o = u.or_else([](int x) { return pf::expected<int, int>(pf::in_place, x); });
  STATIC_REQUIRE(o.has_value());
  STATIC_REQUIRE(*o == 9);

  constexpr auto te = u.transform_error([](int x) { return x + 1; });
  STATIC_REQUIRE(!te.has_value());
  STATIC_REQUIRE(te.error() == 10);
}
