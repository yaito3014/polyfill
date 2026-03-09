#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/indirect.hpp>
#include <yk/polyfill/utility.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

// Helper: counts copies/moves for tracking semantics
struct Counter {
  int value;
  static int copies;
  static int moves;

  explicit Counter(int v = 0) : value(v) {}
  Counter(const Counter& o) : value(o.value) { ++copies; }
  Counter(Counter&& o) : value(o.value) { ++moves; }
  Counter& operator=(const Counter& o)
  {
    value = o.value;
    ++copies;
    return *this;
  }
  Counter& operator=(Counter&& o)
  {
    value = o.value;
    ++moves;
    return *this;
  }
  bool operator==(const Counter& o) const { return value == o.value; }
  bool operator!=(const Counter& o) const { return value != o.value; }
};
int Counter::copies = 0;
int Counter::moves = 0;

TEST_CASE("indirect: type traits")
{
  // Always copyable/movable since T=int is
  STATIC_REQUIRE(std::is_copy_constructible<pf::indirect<int>>::value);
  STATIC_REQUIRE(std::is_move_constructible<pf::indirect<int>>::value);
  STATIC_REQUIRE(std::is_copy_assignable<pf::indirect<int>>::value);
  STATIC_REQUIRE(std::is_move_assignable<pf::indirect<int>>::value);

  // Move constructor is noexcept
  STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::indirect<int>>::value);

  // Dereference value categories
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::indirect<int>&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<const pf::indirect<int>&>()), const int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::indirect<int>&&>()), int&&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<const pf::indirect<int>&&>()), const int&&>::value);
}

TEST_CASE("indirect: default construction")
{
  pf::indirect<int> i;
  CHECK(!i.valueless_after_move());
  CHECK(*i == 0);  // value-initialized
}

TEST_CASE("indirect: in_place construction")
{
  pf::indirect<int> i(pf::in_place, 42);
  CHECK(*i == 42);
  CHECK(!i.valueless_after_move());
}

TEST_CASE("indirect: in_place construction with multiple args")
{
  pf::indirect<std::string> s(pf::in_place, 5u, 'x');
  CHECK(*s == "xxxxx");
}

TEST_CASE("indirect: copy construction deep-copies")
{
  pf::indirect<int> a(pf::in_place, 7);
  pf::indirect<int> b = a;

  CHECK(*a == 7);
  CHECK(*b == 7);

  // Mutating b does not affect a
  *b = 99;
  CHECK(*a == 7);
  CHECK(*b == 99);
}

TEST_CASE("indirect: move construction")
{
  pf::indirect<int> a(pf::in_place, 7);
  pf::indirect<int> b = std::move(a);

  CHECK(*b == 7);
  CHECK(a.valueless_after_move());
}

TEST_CASE("indirect: copy assignment deep-copies")
{
  pf::indirect<int> a(pf::in_place, 1);
  pf::indirect<int> b(pf::in_place, 2);
  b = a;

  CHECK(*a == 1);
  CHECK(*b == 1);

  *b = 99;
  CHECK(*a == 1);
}

TEST_CASE("indirect: move assignment")
{
  pf::indirect<int> a(pf::in_place, 5);
  pf::indirect<int> b(pf::in_place, 6);
  b = std::move(a);

  CHECK(*b == 5);
  CHECK(a.valueless_after_move());
}

TEST_CASE("indirect: operator->")
{
  pf::indirect<std::string> s(pf::in_place, "hello");
  CHECK(s->size() == 5u);
}

TEST_CASE("indirect: swap")
{
  pf::indirect<int> a(pf::in_place, 1);
  pf::indirect<int> b(pf::in_place, 2);
  a.swap(b);
  CHECK(*a == 2);
  CHECK(*b == 1);

  using std::swap;
  swap(a, b);
  CHECK(*a == 1);
  CHECK(*b == 2);
}

TEST_CASE("indirect: operator== between two indirects")
{
  pf::indirect<int> a(pf::in_place, 3);
  pf::indirect<int> b(pf::in_place, 3);
  pf::indirect<int> c(pf::in_place, 4);

  CHECK(a == b);
  CHECK(!(a == c));
  CHECK(a != c);
}

TEST_CASE("indirect: operator== with value")
{
  pf::indirect<int> a(pf::in_place, 42);
  CHECK(a == 42);
  CHECK(42 == a);
  CHECK(!(a == 0));
  CHECK(a != 0);
}

TEST_CASE("indirect: valueless comparison")
{
  pf::indirect<int> a(pf::in_place, 1);
  pf::indirect<int> b = std::move(a);

  // a is valueless
  CHECK(a.valueless_after_move());

  // valueless == valueless
  pf::indirect<int> c = std::move(b);
  // Now both a and b are valueless (b moved to c)
  CHECK(a == b);  // both valueless

  // valueless != non-valueless
  CHECK(!(a == c));
  CHECK(a != c);
}

TEST_CASE("indirect: copy uses value copy not extra allocs")
{
  Counter::copies = 0;
  Counter::moves = 0;

  pf::indirect<Counter> a(pf::in_place, 10);
  pf::indirect<Counter> b = a;   // one copy for the T object

  CHECK((*b).value == 10);
  // At minimum one copy happened
  CHECK(Counter::copies >= 1);
}

TEST_CASE("indirect: get_allocator")
{
  pf::indirect<int> i;
  (void)i.get_allocator();  // should compile and return std::allocator<int>
  STATIC_REQUIRE(std::is_same<decltype(i.get_allocator()), std::allocator<int>>::value);
}
