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

// ---- constraint / mandate tests -------------------------------------------
//
// Constraints (enable_if): affect overload resolution — reflected in type traits.
// Mandates (static_assert): fire at instantiation — NOT reflected in type traits.

struct MoveOnly {
  MoveOnly() = default;
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
};

struct NonDefault {
  explicit NonDefault(int v) : value(v) {}
  NonDefault(const NonDefault&) = default;
  int value;
};

// Allocator with no default constructor — tests the Constraint on indirect().
template <class T>
struct NoDefaultAlloc {
  using value_type = T;
  NoDefaultAlloc() = delete;
  explicit NoDefaultAlloc(int) {}
  template <class U> NoDefaultAlloc(const NoDefaultAlloc<U>&) {}
  T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
  void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p, n); }
  bool operator==(const NoDefaultAlloc&) const { return true; }
  bool operator!=(const NoDefaultAlloc&) const { return false; }
};

// --- Constraint: default ctor requires default-constructible A ---

TEST_CASE("indirect: default ctor Constraint — not in overload set when A has no default ctor")
{
  // Constraint (enable_if) on A: indirect<T, NoDefaultAlloc>() must not be selectable
  STATIC_REQUIRE(!std::is_default_constructible<pf::indirect<int, NoDefaultAlloc<int>>>::value);
}

// --- Constraint: in_place_t ctor requires is_constructible<T, Ts...> ---

TEST_CASE("indirect: in_place_t Constraint — selected when args match")
{
  pf::indirect<std::string> s(pf::in_place, 3u, 'a');
  CHECK(*s == "aaa");
}

TEST_CASE("indirect: in_place_t Constraint — not in overload set when args don't match")
{
  STATIC_REQUIRE(!std::is_constructible<pf::indirect<int>, pf::in_place_t, const char*>::value);
}

// --- Mandate: T must be default-constructible (static_assert, not a type-trait effect) ---

TEST_CASE("indirect: non-default T — in_place construction works")
{
  pf::indirect<NonDefault> p(pf::in_place, 42);
  CHECK(p->value == 42);
}

// --- Mandate: T must be move-constructible (static_assert) ---

TEST_CASE("indirect: move-only T — basic move ctor and move assign work")
{
  pf::indirect<MoveOnly> a(pf::in_place);
  pf::indirect<MoveOnly> b = std::move(a);
  CHECK(a.valueless_after_move());
  CHECK(!b.valueless_after_move());

  pf::indirect<MoveOnly> c(pf::in_place);
  c = std::move(b);
  CHECK(b.valueless_after_move());
}

// --- Generic (converting) constructor ---

TEST_CASE("indirect: generic ctor — constructs from value")
{
  pf::indirect<std::string> s(std::string("hello"));
  CHECK(*s == "hello");
}

TEST_CASE("indirect: generic ctor — constructs from compatible type")
{
  // const char* is constructible to std::string
  pf::indirect<std::string> s("world");
  CHECK(*s == "world");
}

TEST_CASE("indirect: generic ctor — Constraint: not selected when U is incompatible")
{
  // int is not constructible from std::string
  STATIC_REQUIRE(!std::is_constructible<pf::indirect<int>, std::string>::value);
}

TEST_CASE("indirect: generic ctor — Constraint: not in overload set when A has no default ctor")
{
  STATIC_REQUIRE(!std::is_constructible<pf::indirect<int, NoDefaultAlloc<int>>, int>::value);
}

TEST_CASE("indirect: alloc-extended generic ctor — constructs from value with allocator")
{
  std::allocator<std::string> a;
  pf::indirect<std::string> s(std::allocator_arg, a, std::string("hi"));
  CHECK(*s == "hi");
}

// ---- allocator-propagation tests ------------------------------------------
//
// TestAlloc<T, Pocs, Pocca, Pocma> is a minimal stateful allocator with
// selectable propagation traits.  Two instances with different ids compare
// unequal, letting us distinguish allocator-propagation from value-copying.

template <class T, bool Pocs, bool Pocca, bool Pocma>
struct TestAlloc {
  using value_type = T;
  int id;

  template <class U>
  struct rebind { using other = TestAlloc<U, Pocs, Pocca, Pocma>; };

  using propagate_on_container_swap             = std::integral_constant<bool, Pocs>;
  using propagate_on_container_copy_assignment  = std::integral_constant<bool, Pocca>;
  using propagate_on_container_move_assignment  = std::integral_constant<bool, Pocma>;

  explicit TestAlloc(int i = 0) : id(i) {}

  template <class U>
  TestAlloc(const TestAlloc<U, Pocs, Pocca, Pocma>& o) : id(o.id) {}

  T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
  void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p, n); }

  bool operator==(const TestAlloc& o) const { return id == o.id; }
  bool operator!=(const TestAlloc& o) const { return id != o.id; }
};

// Convenience aliases: only the relevant propagation flag is true.
template <class T> using PocswapAlloc = TestAlloc<T, /*Pocs*/true,  false, false>;
template <class T> using PoccaAlloc   = TestAlloc<T, false, /*Pocca*/true,  false>;
template <class T> using PocmaAlloc   = TestAlloc<T, false, false, /*Pocma*/true>;
template <class T> using StaticAlloc  = TestAlloc<T, false, false, false>;

// --- swap ---

TEST_CASE("indirect alloc: swap POCS=false only swaps values")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::indirect<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  x.swap(y);
  CHECK(*x == 20);
  CHECK(*y == 10);
  CHECK(x.get_allocator() == a1);  // allocator NOT propagated
  CHECK(y.get_allocator() == a2);
}

TEST_CASE("indirect alloc: swap POCS=true swaps values and allocators")
{
  PocswapAlloc<int> a1(1), a2(2);
  pf::indirect<int, PocswapAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::indirect<int, PocswapAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  x.swap(y);
  CHECK(*x == 20);
  CHECK(*y == 10);
  CHECK(x.get_allocator() == a2);  // allocator WAS propagated
  CHECK(y.get_allocator() == a1);
}

// --- copy assignment ---

TEST_CASE("indirect alloc: copy assign POCCA=false leaves allocator unchanged")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::indirect<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  y = x;
  CHECK(*y == 10);
  CHECK(y.get_allocator() == a2);  // allocator NOT propagated
}

TEST_CASE("indirect alloc: copy assign POCCA=true propagates allocator")
{
  PoccaAlloc<int> a1(1), a2(2);
  pf::indirect<int, PoccaAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::indirect<int, PoccaAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  y = x;
  CHECK(*y == 10);
  CHECK(y.get_allocator() == a1);  // allocator WAS propagated
}

// --- move assignment ---

TEST_CASE("indirect alloc: move assign POCMA=true steals both value and allocator")
{
  PocmaAlloc<int> a1(1), a2(2);
  pf::indirect<int, PocmaAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::indirect<int, PocmaAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(x.valueless_after_move());
  CHECK(y.get_allocator() == a1);  // allocator WAS propagated (stolen)
}

TEST_CASE("indirect alloc: move assign POCMA=false same alloc steals value")
{
  StaticAlloc<int> a1(1);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::indirect<int, StaticAlloc<int>> y(std::allocator_arg, a1, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(x.valueless_after_move());   // pointer was stolen
  CHECK(y.get_allocator() == a1);
}

TEST_CASE("indirect alloc: move assign POCMA=false different alloc does in-place move")
{
  // Allocs differ and POCMA=false: indirect reuses the destination's allocation
  // and move-assigns through it rather than stealing the pointer.
  StaticAlloc<int> a1(1), a2(2);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::indirect<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(!x.valueless_after_move());  // x keeps its allocation (in-place move, no steal)
  CHECK(y.get_allocator() == a2);    // allocator unchanged
}

// --- extended-allocator move constructor ---

TEST_CASE("indirect alloc: move ctor with same allocator steals pointer")
{
  StaticAlloc<int> a1(1);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 7);
  pf::indirect<int, StaticAlloc<int>> y(std::move(x), std::allocator_arg, a1);
  CHECK(*y == 7);
  CHECK(x.valueless_after_move());
}

TEST_CASE("indirect alloc: move ctor with different allocator copies value")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::indirect<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 7);
  pf::indirect<int, StaticAlloc<int>> y(std::move(x), std::allocator_arg, a2);
  CHECK(*y == 7);
  CHECK(!x.valueless_after_move());  // x was not stolen from
  CHECK(y.get_allocator() == a2);
}
