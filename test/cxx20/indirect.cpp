#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/indirect.hpp>
#include <yk/polyfill/utility.hpp>

namespace pf = yk::polyfill;

// ---- constexpr helpers ----

constexpr int indirect_default_construct()
{
  pf::indirect<int> a;
  return *a;  // value-initialised → 0
}

constexpr int indirect_in_place_construct()
{
  pf::indirect<int> a(pf::in_place, 42);
  return *a;
}

constexpr bool indirect_copy_is_independent()
{
  pf::indirect<int> a(pf::in_place, 7);
  pf::indirect<int> b = a;
  *b = 99;
  return *a == 7 && *b == 99;
}

constexpr bool indirect_move_transfers_ownership()
{
  pf::indirect<int> a(pf::in_place, 5);
  pf::indirect<int> b = static_cast<pf::indirect<int>&&>(a);
  return a.valueless_after_move() && *b == 5;
}

constexpr int indirect_copy_assign()
{
  pf::indirect<int> a(pf::in_place, 3);
  pf::indirect<int> b(pf::in_place, 9);
  b = a;
  return *b;  // 3
}

constexpr bool indirect_move_assign()
{
  pf::indirect<int> a(pf::in_place, 11);
  pf::indirect<int> b(pf::in_place, 22);
  b = static_cast<pf::indirect<int>&&>(a);
  return a.valueless_after_move() && *b == 11;
}

constexpr bool indirect_swap()
{
  pf::indirect<int> a(pf::in_place, 1);
  pf::indirect<int> b(pf::in_place, 2);
  a.swap(b);
  return *a == 2 && *b == 1;
}

#ifndef YK_POLYFILL_DISABLE_COMPARISON_OPS
constexpr bool indirect_eq()
{
  pf::indirect<int> a(pf::in_place, 4);
  pf::indirect<int> b(pf::in_place, 4);
  pf::indirect<int> c(pf::in_place, 5);
  return a == b && !(a == c) && a != c;
}

constexpr bool indirect_valueless_eq()
{
  pf::indirect<int> a(pf::in_place, 1);
  pf::indirect<int> b = static_cast<pf::indirect<int>&&>(a);
  pf::indirect<int> c = static_cast<pf::indirect<int>&&>(b);
  // a and b are both valueless
  return a.valueless_after_move() && b.valueless_after_move()
      && a == b       // valueless == valueless
      && !(a == c);   // valueless != non-valueless
}
#endif  // YK_POLYFILL_DISABLE_COMPARISON_OPS

// ---- test cases ----

TEST_CASE("indirect constexpr: default construction")
{
  STATIC_REQUIRE(indirect_default_construct() == 0);
}

TEST_CASE("indirect constexpr: in_place construction")
{
  STATIC_REQUIRE(indirect_in_place_construct() == 42);
}

TEST_CASE("indirect constexpr: copy is independent")
{
  STATIC_REQUIRE(indirect_copy_is_independent());
}

TEST_CASE("indirect constexpr: move transfers ownership")
{
  STATIC_REQUIRE(indirect_move_transfers_ownership());
}

TEST_CASE("indirect constexpr: copy assignment")
{
  STATIC_REQUIRE(indirect_copy_assign() == 3);
}

TEST_CASE("indirect constexpr: move assignment")
{
  STATIC_REQUIRE(indirect_move_assign());
}

TEST_CASE("indirect constexpr: swap")
{
  STATIC_REQUIRE(indirect_swap());
}

#ifndef YK_POLYFILL_DISABLE_COMPARISON_OPS
TEST_CASE("indirect constexpr: equality comparison")
{
  STATIC_REQUIRE(indirect_eq());
}

TEST_CASE("indirect constexpr: valueless equality")
{
  STATIC_REQUIRE(indirect_valueless_eq());
}
#endif  // YK_POLYFILL_DISABLE_COMPARISON_OPS
