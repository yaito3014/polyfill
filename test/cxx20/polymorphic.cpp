#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/polymorphic.hpp>
#include <yk/polyfill/utility.hpp>

namespace pf = yk::polyfill;

// ---- constexpr helpers ----

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

// ---- test cases ----

TEST_CASE("polymorphic constexpr: default construction")
{
  STATIC_REQUIRE(polymorphic_default_construct() == 0);
}

TEST_CASE("polymorphic constexpr: in_place construction")
{
  STATIC_REQUIRE(polymorphic_in_place_construct() == 42);
}

TEST_CASE("polymorphic constexpr: copy is independent")
{
  STATIC_REQUIRE(polymorphic_copy_is_independent());
}

TEST_CASE("polymorphic constexpr: move transfers ownership")
{
  STATIC_REQUIRE(polymorphic_move_transfers_ownership());
}

TEST_CASE("polymorphic constexpr: copy assignment")
{
  STATIC_REQUIRE(polymorphic_copy_assign() == 3);
}

TEST_CASE("polymorphic constexpr: move assignment")
{
  STATIC_REQUIRE(polymorphic_move_assign());
}

TEST_CASE("polymorphic constexpr: swap")
{
  STATIC_REQUIRE(polymorphic_swap());
}

TEST_CASE("polymorphic constexpr: equality comparison")
{
  STATIC_REQUIRE(polymorphic_eq());
}

TEST_CASE("polymorphic constexpr: valueless equality")
{
  STATIC_REQUIRE(polymorphic_valueless_eq());
}
