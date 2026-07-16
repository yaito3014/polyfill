#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/expected.hpp>
#include <yk/polyfill/utility.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

namespace {

// literal type with a non-trivial (constexpr) destructor, so expected uses the non-trivial
// storage path and its member-changing operations are exercised in a constant expression.
struct NonTrivial {
  int value;
  constexpr NonTrivial(int v = 0) noexcept : value(v) {}
  constexpr NonTrivial(NonTrivial const&) = default;
  constexpr NonTrivial(NonTrivial&&) = default;
  constexpr NonTrivial& operator=(NonTrivial const&) = default;
  constexpr NonTrivial& operator=(NonTrivial&&) = default;
  constexpr ~NonTrivial() {}
  constexpr bool operator==(NonTrivial const& other) const { return value == other.value; }
};

}  // namespace

TEST_CASE("expected constexpr with non-trivial type (C++20)")
{
  STATIC_REQUIRE_FALSE(std::is_trivially_destructible<pf::expected<NonTrivial, int>>::value);

  constexpr auto construct = []() {
    pf::expected<NonTrivial, int> e(pf::in_place, 5);
    return e.has_value() && e->value == 5;
  };
  STATIC_REQUIRE(construct());

  constexpr auto assign_arm_switch = []() {
    pf::expected<NonTrivial, int> e(pf::in_place, 1);
    e = pf::unexpected<int>(9);  // value -> error (reinit)
    bool ok1 = !e.has_value() && e.error() == 9;
    e = NonTrivial(2);  // error -> value (reinit)
    bool ok2 = e.has_value() && e->value == 2;
    return ok1 && ok2;
  };
  STATIC_REQUIRE(assign_arm_switch());

  constexpr auto emplace = []() {
    pf::expected<NonTrivial, int> e(pf::unexpect, 1);
    e.emplace(7);
    return e.has_value() && e->value == 7;
  };
  STATIC_REQUIRE(emplace());

  constexpr auto do_swap = []() {
    pf::expected<NonTrivial, int> a(pf::in_place, 1);
    pf::expected<NonTrivial, int> b(pf::unexpect, 2);
    a.swap(b);
    return !a.has_value() && a.error() == 2 && b.has_value() && b->value == 1;
  };
  STATIC_REQUIRE(do_swap());

  constexpr auto void_reinit = []() {
    pf::expected<void, NonTrivial> e;
    e = pf::unexpected<NonTrivial>(NonTrivial(3));  // value -> error
    bool ok1 = !e.has_value() && e.error().value == 3;
    e.emplace();  // error -> value
    return ok1 && e.has_value();
  };
  STATIC_REQUIRE(void_reinit());
}

TEST_CASE("expected != is synthesized from == (C++20)")
{
  pf::expected<int, int> a(pf::in_place, 1);
  pf::expected<int, int> b(pf::in_place, 2);
  pf::expected<int, int> e(pf::unexpect, 5);

  CHECK(a != b);
  CHECK(!(a != pf::expected<int, int>(pf::in_place, 1)));
  CHECK(a != e);

  // reversed / rewritten candidates
  CHECK(a != 2);
  CHECK(2 != a);
  CHECK(a == 1);
  CHECK(1 == a);
  CHECK(e != pf::unexpected<int>(4));
  CHECK(pf::unexpected<int>(4) != e);
  CHECK(e == pf::unexpected<int>(5));

  pf::expected<void, int> v;
  pf::expected<void, int> w(pf::unexpect, 1);
  CHECK(v != w);
  CHECK(!(v != pf::expected<void, int>()));
}

TEST_CASE("expected constexpr comparisons (C++20)")
{
  constexpr pf::expected<int, int> a(pf::in_place, 1);
  constexpr pf::expected<int, int> b(pf::unexpect, 2);
  STATIC_REQUIRE(a != b);
  STATIC_REQUIRE(a == 1);
  STATIC_REQUIRE(b == pf::unexpected<int>(2));
}
