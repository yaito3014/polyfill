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

// neither copyable nor movable; transform/transform_error must still produce it in place
struct NoMove {
  int v;
  explicit NoMove(int x) : v(x) {}
  NoMove(NoMove const&) = delete;
  NoMove(NoMove&&) = delete;
};

struct Point {
  int x;
};

// Swappable per [swappable.requirements] through the base class swap, although ranges::swap
// rejects it. This needs C++17, where std::swap is constrained on move-constructibility.
struct id_base {
  int id;
};

void swap(id_base& a, id_base& b) noexcept
{
  int const tmp = a.id;
  a.id = b.id;
  b.id = tmp;
}

struct base_swap_only : id_base {
  explicit base_swap_only(int i) : id_base{i} {}
  base_swap_only(base_swap_only&&) = delete;
};

}  // namespace

TEST_CASE("unexpected swap of a non-movable error swappable through a base class")
{
  STATIC_REQUIRE(pf::is_swappable<base_swap_only>::value);
  pf::unexpected<base_swap_only> a(pf::in_place, 1), b(pf::in_place, 2);
  a.swap(b);
  CHECK(a.error().id == 2);
  CHECK(b.error().id == 1);
  swap(a, b);
  CHECK(a.error().id == 1);
  CHECK(b.error().id == 2);
}

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

TEST_CASE("expected transform with non-movable result")
{
  pf::expected<int, int> e(pf::in_place, 1);
  auto r = e.transform([](int i) { return NoMove(i + 1); });
  CHECK(r->v == 2);
  auto rr = std::move(e).transform([](int i) { return NoMove(i + 2); });
  CHECK(rr->v == 3);

  pf::expected<int, int> u(pf::unexpect, 4);
  auto re = u.transform_error([](int i) { return NoMove(i); });
  CHECK(re.error().v == 4);
  auto rre = std::move(u).transform_error([](int i) { return NoMove(i + 1); });
  CHECK(rre.error().v == 5);

  pf::expected<void, int> ve;
  auto rv = ve.transform([] { return NoMove(6); });
  CHECK(rv->v == 6);

  pf::expected<void, int> vu(pf::unexpect, 7);
  auto rve = vu.transform_error([](int i) { return NoMove(i); });
  CHECK(rve.error().v == 7);
}

TEST_CASE("expected operator-> is constexpr from C++17")
{
  // std::addressof is constexpr only from C++17
  constexpr pf::expected<Point, int> e(pf::in_place, Point{3});
  STATIC_REQUIRE(e->x == 3);
  STATIC_REQUIRE([] {
    pf::expected<Point, int> m(pf::in_place, Point{5});
    return m->x;
  }() == 5);
}
