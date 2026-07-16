#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/expected.hpp>
#include <yk/polyfill/utility.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

struct NeedExplicit {
  int value;
  explicit NeedExplicit(int arg) : value(arg) {}
};

struct CountedDtor {
  static int count;
  ~CountedDtor() { ++count; }
};
int CountedDtor::count = 0;

struct FromInt {
  int v;
  FromInt(int x) : v(x) {}
};

int times_two(int x) { return 2 * x; }

// copy/move constructible + assignable, but with a potentially-throwing move constructor.
struct ThrowMove {
  int tag = 0;
  ThrowMove() = default;
  ThrowMove(int t) : tag(t) {}
  ThrowMove(ThrowMove const&) = default;
  ThrowMove(ThrowMove&&) noexcept(false) { throw std::runtime_error("move"); }
  ThrowMove& operator=(ThrowMove const&) = default;
  ThrowMove& operator=(ThrowMove&&) = default;
};

// throws from copy/move only while armed; move is non-noexcept so reinit uses the backup rung.
struct Bomb {
  int tag = 0;
  static bool armed;
  Bomb() = default;
  Bomb(int t) : tag(t) {}
  Bomb(Bomb const& o) : tag(o.tag)
  {
    if (armed) throw std::runtime_error("copy");
  }
  Bomb(Bomb&& o) noexcept(false) : tag(o.tag)
  {
    if (armed) throw std::runtime_error("move");
  }
  Bomb& operator=(Bomb const& o) = default;
};
bool Bomb::armed = false;

}  // namespace

TEST_CASE("unexpected basics")
{
  // construction from value + error() accessors
  pf::unexpected<int> u(5);
  CHECK(u.error() == 5);

  pf::unexpected<int> const cu(7);
  CHECK(cu.error() == 7);
  STATIC_REQUIRE(std::is_same<decltype(cu.error()), int const&>::value);
  STATIC_REQUIRE(std::is_same<decltype(pf::unexpected<int>(1).error()), int&&>::value);

  // in_place construction
  pf::unexpected<std::string> us(pf::in_place, 3u, 'a');
  CHECK(us.error() == "aaa");

  // equality
  CHECK(pf::unexpected<int>(3) == pf::unexpected<int>(3));
  CHECK(pf::unexpected<int>(3) != pf::unexpected<long>(4));

  // swap
  pf::unexpected<int> a(1), b(2);
  a.swap(b);
  CHECK(a.error() == 2);
  CHECK(b.error() == 1);
  swap(a, b);
  CHECK(a.error() == 1);
  CHECK(b.error() == 2);
}

TEST_CASE("expected triviality")
{
  STATIC_REQUIRE(std::is_trivially_copy_constructible<pf::expected<int, int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_constructible<pf::expected<int, int>>::value);
  STATIC_REQUIRE(std::is_trivially_copy_assignable<pf::expected<int, int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_assignable<pf::expected<int, int>>::value);
  STATIC_REQUIRE(std::is_trivially_destructible<pf::expected<int, int>>::value);

  STATIC_REQUIRE(std::is_trivially_copy_constructible<pf::expected<void, int>>::value);
  STATIC_REQUIRE(std::is_trivially_destructible<pf::expected<void, int>>::value);

  STATIC_REQUIRE_FALSE(std::is_trivially_destructible<pf::expected<std::string, int>>::value);
  STATIC_REQUIRE_FALSE(std::is_trivially_destructible<pf::expected<int, std::string>>::value);
}

TEST_CASE("expected member types")
{
  STATIC_REQUIRE(std::is_same<pf::expected<int, long>::value_type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::expected<int, long>::error_type, long>::value);
  STATIC_REQUIRE(std::is_same<pf::expected<int, long>::unexpected_type, pf::unexpected<long>>::value);
  STATIC_REQUIRE(std::is_same<pf::expected<int, long>::rebind<char>, pf::expected<char, long>>::value);
  STATIC_REQUIRE(std::is_same<pf::expected<void, long>::value_type, void>::value);
}

TEST_CASE("expected construction")
{
  // default (value-initialized T)
  {
    pf::expected<int, std::string> e;
    CHECK(e.has_value());
    CHECK(*e == 0);
  }

  // in_place
  {
    pf::expected<int, std::string> e(pf::in_place, 42);
    CHECK(e.has_value());
    CHECK(*e == 42);
  }

  // unexpect
  {
    pf::expected<int, std::string> e(pf::unexpect, 3u, 'z');
    CHECK(!e.has_value());
    CHECK(e.error() == "zzz");
  }

  // value forwarding
  {
    pf::expected<int, std::string> e = 7;
    CHECK(e.has_value());
    CHECK(*e == 7);
  }

  // from unexpected
  {
    pf::expected<int, std::string> e = pf::unexpected<std::string>("boom");
    CHECK(!e.has_value());
    CHECK(e.error() == "boom");
  }

  // explicit value conversion
  {
    pf::expected<NeedExplicit, int> e(pf::in_place, 3);
    CHECK(e.has_value());
    CHECK(e->value == 3);
    STATIC_REQUIRE_FALSE(std::is_convertible<int, pf::expected<NeedExplicit, int>>::value);
    STATIC_REQUIRE(std::is_constructible<pf::expected<NeedExplicit, int>, int>::value);
  }
}

TEST_CASE("expected converting construction")
{
  // expected<U,G> const& -> expected<T,E>
  {
    pf::expected<int, int> src(pf::in_place, 5);
    pf::expected<long, long> dst(src);
    CHECK(dst.has_value());
    CHECK(*dst == 5);
  }
  {
    pf::expected<int, int> src(pf::unexpect, 9);
    pf::expected<long, long> dst(src);
    CHECK(!dst.has_value());
    CHECK(dst.error() == 9);
  }
  // move
  {
    pf::expected<FromInt, int> dst(pf::expected<int, int>(pf::in_place, 8));
    CHECK(dst.has_value());
    CHECK(dst->v == 8);
  }
}

TEST_CASE("expected observers and value()")
{
  pf::expected<int, std::string> e(pf::in_place, 10);
  CHECK(static_cast<bool>(e));
  CHECK(e.value() == 10);
  CHECK(*e == 10);
  CHECK(e.value_or(99) == 10);

  pf::expected<int, std::string> u(pf::unexpect, "err");
  CHECK(!static_cast<bool>(u));
  CHECK(u.error() == "err");
  CHECK(u.value_or(99) == 99);
  CHECK(u.error_or(std::string("x")) == "err");
  CHECK(e.error_or(std::string("x")) == "x");

  REQUIRE_THROWS_AS(u.value(), pf::bad_expected_access<std::string>);

  // the thrown exception carries the error
  bool caught = false;
  try {
    u.value();
  } catch (pf::bad_expected_access<std::string> const& ex) {
    caught = true;
    CHECK(ex.error() == "err");
  }
  CHECK(caught);

  // base bad_expected_access<void> catch
  bool caught_base = false;
  try {
    u.value();
  } catch (pf::bad_expected_access<void> const&) {
    caught_base = true;
  }
  CHECK(caught_base);
}

TEST_CASE("expected assignment")
{
  // value = value
  {
    pf::expected<int, std::string> e(pf::in_place, 1);
    e = 2;
    CHECK(e.has_value());
    CHECK(*e == 2);
  }
  // error -> value (arm switch)
  {
    pf::expected<int, std::string> e(pf::unexpect, "e");
    e = 5;
    CHECK(e.has_value());
    CHECK(*e == 5);
  }
  // value -> error (arm switch)
  {
    pf::expected<int, std::string> e(pf::in_place, 1);
    e = pf::unexpected<std::string>("bad");
    CHECK(!e.has_value());
    CHECK(e.error() == "bad");
  }
  // error = error
  {
    pf::expected<int, std::string> e(pf::unexpect, "a");
    e = pf::unexpected<std::string>("b");
    CHECK(!e.has_value());
    CHECK(e.error() == "b");
  }
  // copy assignment across arms
  {
    pf::expected<int, std::string> a(pf::in_place, 1);
    pf::expected<int, std::string> b(pf::unexpect, "z");
    a = b;
    CHECK(!a.has_value());
    CHECK(a.error() == "z");
  }
  // move assignment across arms
  {
    pf::expected<std::string, int> a(pf::unexpect, 3);
    pf::expected<std::string, int> b(pf::in_place, "hello");
    a = std::move(b);
    CHECK(a.has_value());
    CHECK(*a == "hello");
  }
}

TEST_CASE("expected emplace")
{
  pf::expected<int, std::string> e(pf::unexpect, "e");
  int& r = e.emplace(77);
  CHECK(e.has_value());
  CHECK(*e == 77);
  CHECK(&r == &*e);
}

TEST_CASE("expected swap")
{
  // value <-> error
  {
    pf::expected<int, std::string> a(pf::in_place, 1);
    pf::expected<int, std::string> b(pf::unexpect, "err");
    a.swap(b);
    CHECK(!a.has_value());
    CHECK(a.error() == "err");
    CHECK(b.has_value());
    CHECK(*b == 1);
  }
  // value <-> value via free swap
  {
    pf::expected<int, std::string> a(pf::in_place, 1);
    pf::expected<int, std::string> b(pf::in_place, 2);
    swap(a, b);
    CHECK(*a == 2);
    CHECK(*b == 1);
  }
  // error <-> error
  {
    pf::expected<int, std::string> a(pf::unexpect, "a");
    pf::expected<int, std::string> b(pf::unexpect, "b");
    a.swap(b);
    CHECK(a.error() == "b");
    CHECK(b.error() == "a");
  }
}

TEST_CASE("expected monadic and_then / or_else")
{
  pf::expected<int, std::string> e(pf::in_place, 4);

  auto r = e.and_then([](int x) { return pf::expected<int, std::string>(pf::in_place, x + 1); });
  CHECK(r.has_value());
  CHECK(*r == 5);

  pf::expected<int, std::string> u(pf::unexpect, "no");
  auto r2 = u.and_then([](int x) { return pf::expected<int, std::string>(pf::in_place, x + 1); });
  CHECK(!r2.has_value());
  CHECK(r2.error() == "no");

  auto r3 = u.or_else([](std::string const& s) { return pf::expected<int, std::string>(pf::in_place, static_cast<int>(s.size())); });
  CHECK(r3.has_value());
  CHECK(*r3 == 2);

  auto r4 = e.or_else([](std::string const&) { return pf::expected<int, std::string>(pf::unexpect, "x"); });
  CHECK(r4.has_value());
  CHECK(*r4 == 4);
}

TEST_CASE("expected monadic transform / transform_error")
{
  pf::expected<int, std::string> e(pf::in_place, 4);

  auto r = e.transform([](int x) { return x * 10; });
  STATIC_REQUIRE(std::is_same<decltype(r), pf::expected<int, std::string>>::value);
  CHECK(r.has_value());
  CHECK(*r == 40);

  // transform to void
  auto rv = e.transform([](int) {});
  STATIC_REQUIRE(std::is_same<decltype(rv), pf::expected<void, std::string>>::value);
  CHECK(rv.has_value());

  pf::expected<int, std::string> u(pf::unexpect, "oops");
  auto re = u.transform_error([](std::string const& s) { return static_cast<int>(s.size()); });
  STATIC_REQUIRE(std::is_same<decltype(re), pf::expected<int, int>>::value);
  CHECK(!re.has_value());
  CHECK(re.error() == 4);
}

TEST_CASE("expected comparisons")
{
  pf::expected<int, int> a(pf::in_place, 1);
  pf::expected<int, int> b(pf::in_place, 1);
  pf::expected<int, int> c(pf::in_place, 2);
  pf::expected<int, int> e(pf::unexpect, 9);

  CHECK(a == b);
  CHECK(a != c);
  CHECK(a != e);
  CHECK(a == 1);
  CHECK(a != 2);
  CHECK(e == pf::unexpected<int>(9));
  CHECK(e != pf::unexpected<int>(8));
  CHECK(1 == a);
  CHECK(pf::unexpected<int>(9) == e);
}

TEST_CASE("expected<void, E>")
{
  // default
  {
    pf::expected<void, int> e;
    CHECK(e.has_value());
    CHECK(static_cast<bool>(e));
    e.value();  // no throw
  }
  // in_place
  {
    pf::expected<void, int> e(pf::in_place);
    CHECK(e.has_value());
  }
  // unexpect
  {
    pf::expected<void, int> e(pf::unexpect, 3);
    CHECK(!e.has_value());
    CHECK(e.error() == 3);
    REQUIRE_THROWS_AS(e.value(), pf::bad_expected_access<int>);
    CHECK(e.error_or(7) == 3);
  }
  // from unexpected
  {
    pf::expected<void, int> e = pf::unexpected<int>(5);
    CHECK(!e.has_value());
    CHECK(e.error() == 5);
  }
  // assignment / emplace
  {
    pf::expected<void, int> e(pf::unexpect, 1);
    e = pf::unexpected<int>(2);
    CHECK(e.error() == 2);
    e.emplace();
    CHECK(e.has_value());
  }
  // swap
  {
    pf::expected<void, int> a;
    pf::expected<void, int> b(pf::unexpect, 4);
    a.swap(b);
    CHECK(!a.has_value());
    CHECK(a.error() == 4);
    CHECK(b.has_value());
  }
  // comparisons
  {
    pf::expected<void, int> a;
    pf::expected<void, int> b;
    pf::expected<void, int> e(pf::unexpect, 1);
    CHECK(a == b);
    CHECK(a != e);
    CHECK(e == pf::unexpected<int>(1));
  }
  // monadic
  {
    pf::expected<void, int> a;
    auto r = a.and_then([]() { return pf::expected<int, int>(pf::in_place, 5); });
    CHECK(r.has_value());
    CHECK(*r == 5);

    auto t = a.transform([]() { return 42; });
    STATIC_REQUIRE(std::is_same<decltype(t), pf::expected<int, int>>::value);
    CHECK(*t == 42);

    pf::expected<void, int> u(pf::unexpect, 8);
    auto o = u.or_else([](int x) { return pf::expected<void, int>(pf::unexpect, x + 1); });
    CHECK(!o.has_value());
    CHECK(o.error() == 9);
  }
}

TEST_CASE("expected assignment SMF deletion")
{
  // both arms nothrow-move-constructible -> copy/move assignment available
  STATIC_REQUIRE(std::is_copy_assignable<pf::expected<int, std::string>>::value);
  STATIC_REQUIRE(std::is_move_assignable<pf::expected<int, std::string>>::value);

  // neither arm is nothrow-move-constructible -> [expected.object.assign] deletes assignment,
  // beyond what copy/move constructible+assignable alone would allow.
  STATIC_REQUIRE(std::is_copy_constructible<ThrowMove>::value);
  STATIC_REQUIRE(std::is_copy_assignable<ThrowMove>::value);
  STATIC_REQUIRE_FALSE(std::is_nothrow_move_constructible<ThrowMove>::value);
  STATIC_REQUIRE_FALSE(std::is_copy_assignable<pf::expected<ThrowMove, ThrowMove>>::value);
  STATIC_REQUIRE_FALSE(std::is_move_assignable<pf::expected<ThrowMove, ThrowMove>>::value);

  // but if one arm is nothrow-move-constructible, assignment is restored
  STATIC_REQUIRE(std::is_copy_assignable<pf::expected<ThrowMove, int>>::value);
}

TEST_CASE("expected swap constraints")
{
  STATIC_REQUIRE(pf::is_swappable<pf::expected<int, std::string>>::value);
  STATIC_REQUIRE(pf::is_swappable<pf::expected<void, int>>::value);

  // [expected.object.swap] also requires
  // (is_nothrow_move_constructible<T> || is_nothrow_move_constructible<E>).
  // Neither arm qualifies here, so swap must drop out rather than hard-error.
  STATIC_REQUIRE_FALSE(pf::is_swappable<pf::expected<ThrowMove, ThrowMove>>::value);

  // one nothrow-move-constructible arm restores swappability
  STATIC_REQUIRE(pf::is_swappable<pf::expected<ThrowMove, int>>::value);
}

TEST_CASE("expected converting construction with bool value type")
{
  // [expected.object.cons] applies the converts-from-any-cvref check only "if T is not cv bool",
  // so expected<bool, E> stays constructible from another expected despite operator bool.
  STATIC_REQUIRE(std::is_constructible<pf::expected<bool, int>, pf::expected<int, int>>::value);

  pf::expected<int, int> src(pf::in_place, 1);
  pf::expected<bool, int> dst(src);
  CHECK(dst.has_value());
  CHECK(*dst == true);

  pf::expected<int, int> err(pf::unexpect, 4);
  pf::expected<bool, int> derr(err);
  CHECK(!derr.has_value());
  CHECK(derr.error() == 4);
}

TEST_CASE("expected reinit strong guarantee on throwing arm switch")
{
  pf::expected<Bomb, int> a(pf::unexpect, 7);
  pf::expected<Bomb, int> b(pf::in_place, Bomb(5));

  Bomb::armed = true;
  bool threw = false;
  try {
    a = b;  // switch error -> value; the value copy throws, old error must be restored
  } catch (std::runtime_error const&) {
    threw = true;
  }
  Bomb::armed = false;

  CHECK(threw);
  CHECK(!a.has_value());
  CHECK(a.error() == 7);
}

TEST_CASE("expected move-only value")
{
  pf::expected<std::unique_ptr<int>, int> e(pf::in_place, new int(5));
  CHECK(e.has_value());
  CHECK(*e.value() == 5);

  pf::expected<std::unique_ptr<int>, int> moved(std::move(e));
  CHECK(moved.has_value());
  CHECK(*moved.value() == 5);

  pf::expected<std::unique_ptr<int>, int> err(pf::unexpect, 9);
  moved = std::move(err);
  CHECK(!moved.has_value());
  CHECK(moved.error() == 9);
}

TEST_CASE("expected rvalue observers")
{
  // value() rvalue overloads
  {
    pf::expected<std::string, int> e(pf::in_place, "hello");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).value()), std::string&&>::value);
    std::string s = std::move(e).value();
    CHECK(s == "hello");
  }
  {
    pf::expected<std::string, int> const e(pf::in_place, "world");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).value()), std::string const&&>::value);
  }

  // value() rvalue throws with moved error
  {
    pf::expected<int, std::string> u(pf::unexpect, "rval-err");
    bool caught = false;
    try {
      std::move(u).value();
    } catch (pf::bad_expected_access<std::string> const& ex) {
      caught = true;
      CHECK(ex.error() == "rval-err");
    }
    CHECK(caught);
  }

  // error() rvalue overloads
  {
    pf::expected<int, std::string> e(pf::unexpect, "err");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).error()), std::string&&>::value);
    std::string s = std::move(e).error();
    CHECK(s == "err");
  }
  {
    pf::expected<int, std::string> const e(pf::unexpect, "cerr");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).error()), std::string const&&>::value);
  }

  // operator* rvalue overloads
  {
    pf::expected<std::string, int> e(pf::in_place, "deref");
    STATIC_REQUIRE(std::is_same<decltype(*std::move(e)), std::string&&>::value);
    std::string s = *std::move(e);
    CHECK(s == "deref");
  }
  {
    pf::expected<std::string, int> const e(pf::in_place, "cderef");
    STATIC_REQUIRE(std::is_same<decltype(*std::move(e)), std::string const&&>::value);
  }
}

TEST_CASE("expected<void> rvalue observers")
{
  // value() rvalue on void (just must not throw)
  {
    pf::expected<void, int> e;
    std::move(e).value();
  }

  // value() rvalue throws with moved error
  {
    pf::expected<void, std::string> u(pf::unexpect, "void-rval");
    bool caught = false;
    try {
      std::move(u).value();
    } catch (pf::bad_expected_access<std::string> const& ex) {
      caught = true;
      CHECK(ex.error() == "void-rval");
    }
    CHECK(caught);
  }

  // error() rvalue overloads
  {
    pf::expected<void, std::string> e(pf::unexpect, "verr");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).error()), std::string&&>::value);
    std::string s = std::move(e).error();
    CHECK(s == "verr");
  }
  {
    pf::expected<void, std::string> const e(pf::unexpect, "vcerr");
    STATIC_REQUIRE(std::is_same<decltype(std::move(e).error()), std::string const&&>::value);
  }
}

TEST_CASE("expected transform with function pointer")
{
  pf::expected<int, std::string> e(pf::in_place, 5);
  auto r = e.transform(times_two);
  CHECK(r.has_value());
  CHECK(*r == 10);

  pf::expected<int, std::string> u(pf::unexpect, "nope");
  auto r2 = u.transform(times_two);
  CHECK(!r2.has_value());
  CHECK(r2.error() == "nope");
}

TEST_CASE("expected monadic rvalue overloads")
{
  // and_then on rvalue
  {
    pf::expected<std::string, int> e(pf::in_place, "hi");
    auto r = std::move(e).and_then([](std::string&& s) { return pf::expected<std::size_t, int>(pf::in_place, s.size()); });
    CHECK(r.has_value());
    CHECK(*r == 2u);
  }
  // or_else on rvalue
  {
    pf::expected<int, std::string> u(pf::unexpect, "err");
    auto r = std::move(u).or_else([](std::string&& s) { return pf::expected<int, std::string>(pf::in_place, static_cast<int>(s.size())); });
    CHECK(r.has_value());
    CHECK(*r == 3);
  }
  // transform on rvalue
  {
    pf::expected<std::string, int> e(pf::in_place, "test");
    auto r = std::move(e).transform([](std::string&& s) { return s.size(); });
    CHECK(r.has_value());
    CHECK(*r == 4u);
  }
  // transform_error on rvalue
  {
    pf::expected<int, std::string> u(pf::unexpect, "boom");
    auto r = std::move(u).transform_error([](std::string&& s) { return s.size(); });
    CHECK(!r.has_value());
    CHECK(r.error() == 4u);
  }
}
