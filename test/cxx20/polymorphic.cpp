#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/polymorphic.hpp>
#include <yk/polyfill/utility.hpp>

#include <compare>

// VS2022 (MSVC < 19.50) cannot evaluate polymorphic in a constant expression
// because destroy() is not constexpr there (compiler bug workaround).
#if defined(_MSC_VER) && _MSC_VER < 1950
#define POLYMORPHIC_STATIC_REQUIRE(x) REQUIRE(x)
#else
#define POLYMORPHIC_STATIC_REQUIRE(x) STATIC_REQUIRE(x)
#endif

namespace pf = yk::polyfill;

// ---- simple constexpr hierarchy for derived-type tests ----

struct ConstBase {
  constexpr virtual ~ConstBase() = default;
  constexpr virtual int value() const = 0;
  constexpr bool operator==(const ConstBase& o) const { return value() == o.value(); }
};

struct ConstDerived : ConstBase {
  int val;
  constexpr explicit ConstDerived(int v) : val(v) {}
  constexpr int value() const override { return val; }
};

constexpr int polymorphic_default_construct()
{
  pf::polymorphic<int> a;
  return *a;  // value-initialised → 0
}

constexpr int polymorphic_in_place_construct()
{
  pf::polymorphic<int> a(pf::in_place, 42);
  return *a;
}

constexpr bool polymorphic_copy_is_independent()
{
  pf::polymorphic<int> a(pf::in_place, 7);
  pf::polymorphic<int> b = a;
  *b = 99;
  return *a == 7 && *b == 99;
}

constexpr bool polymorphic_move_transfers_ownership()
{
  pf::polymorphic<int> a(pf::in_place, 5);
  pf::polymorphic<int> b = static_cast<pf::polymorphic<int>&&>(a);
  return a.valueless_after_move() && *b == 5;
}

constexpr int polymorphic_copy_assign()
{
  pf::polymorphic<int> a(pf::in_place, 3);
  pf::polymorphic<int> b(pf::in_place, 9);
  b = a;
  return *b;  // 3
}

constexpr bool polymorphic_move_assign()
{
  pf::polymorphic<int> a(pf::in_place, 11);
  pf::polymorphic<int> b(pf::in_place, 22);
  b = static_cast<pf::polymorphic<int>&&>(a);
  return a.valueless_after_move() && *b == 11;
}

constexpr bool polymorphic_swap()
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b(pf::in_place, 2);
  a.swap(b);
  return *a == 2 && *b == 1;
}

constexpr bool polymorphic_eq()
{
  pf::polymorphic<int> a(pf::in_place, 4);
  pf::polymorphic<int> b(pf::in_place, 4);
  pf::polymorphic<int> c(pf::in_place, 5);
  return a == b && !(a == c) && a != c;
}

constexpr bool polymorphic_valueless_eq()
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b = static_cast<pf::polymorphic<int>&&>(a);
  pf::polymorphic<int> c = static_cast<pf::polymorphic<int>&&>(b);
  return a.valueless_after_move() && b.valueless_after_move()
      && a == b
      && !(a == c);
}

TEST_CASE("polymorphic constexpr: default construction")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_default_construct() == 0);
}

TEST_CASE("polymorphic constexpr: in_place construction")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_in_place_construct() == 42);
}

TEST_CASE("polymorphic constexpr: copy is independent")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_copy_is_independent());
}

TEST_CASE("polymorphic constexpr: move transfers ownership")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_move_transfers_ownership());
}

TEST_CASE("polymorphic constexpr: copy assignment")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_copy_assign() == 3);
}

TEST_CASE("polymorphic constexpr: move assignment")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_move_assign());
}

TEST_CASE("polymorphic constexpr: swap")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_swap());
}

TEST_CASE("polymorphic constexpr: equality comparison")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_eq());
}

TEST_CASE("polymorphic constexpr: valueless equality")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_valueless_eq());
}

constexpr bool polymorphic_in_place_type_stores_derived()
{
  pf::polymorphic<ConstBase> p(pf::in_place_type<ConstDerived>, 42);
  return p->value() == 42;
}

constexpr bool polymorphic_copy_preserves_dynamic_type()
{
  pf::polymorphic<ConstBase> p1(pf::in_place_type<ConstDerived>, 7);
  pf::polymorphic<ConstBase> p2 = p1;
  return p2->value() == 7;
}

TEST_CASE("polymorphic constexpr: in_place_type stores derived")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_in_place_type_stores_derived());
}

TEST_CASE("polymorphic constexpr: copy preserves dynamic type")
{
  POLYMORPHIC_STATIC_REQUIRE(polymorphic_copy_preserves_dynamic_type());
}

TEST_CASE("polymorphic: spaceship comparison between two polymorphics")
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b(pf::in_place, 2);
  pf::polymorphic<int> c(pf::in_place, 1);

  REQUIRE((a <=> c) == std::strong_ordering::equal);
  REQUIRE((a <=> b) == std::strong_ordering::less);
  REQUIRE((b <=> a) == std::strong_ordering::greater);
}

TEST_CASE("polymorphic: spaceship comparison with raw value")
{
  pf::polymorphic<int> p(pf::in_place, 5);

  REQUIRE((p <=> 5) == std::strong_ordering::equal);
  REQUIRE((p <=> 6) == std::strong_ordering::less);
  REQUIRE((p <=> 4) == std::strong_ordering::greater);
}

TEST_CASE("polymorphic: valueless spaceship comparison")
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b = std::move(a);

  REQUIRE((a <=> b) == std::strong_ordering::less);
  REQUIRE((b <=> a) == std::strong_ordering::greater);

  pf::polymorphic<int> c = std::move(b);
  REQUIRE((a <=> b) == std::strong_ordering::equal);
}
