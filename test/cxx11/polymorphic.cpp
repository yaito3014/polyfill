#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/polymorphic.hpp>
#include <yk/polyfill/utility.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

// ---- Base/Derived hierarchy for polymorphic tests ----

struct Animal {
  virtual ~Animal() = default;
  virtual std::string sound() const = 0;
  virtual Animal* clone_self() const = 0;  // manual clone for verification

  bool operator==(const Animal& o) const { return sound() == o.sound(); }
  bool operator!=(const Animal& o) const { return !(*this == o); }
};

struct Dog : Animal {
  std::string sound() const override { return "woof"; }
  Animal* clone_self() const override { return new Dog(*this); }
};

struct Cat : Animal {
  std::string sound() const override { return "meow"; }
  Animal* clone_self() const override { return new Cat(*this); }
};

// Simple non-polymorphic type for basic tests
struct Point {
  int x, y;
  explicit Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
  bool operator==(const Point& o) const { return x == o.x && y == o.y; }
  bool operator!=(const Point& o) const { return !(*this == o); }
};

TEST_CASE("polymorphic: type traits")
{
  STATIC_REQUIRE(std::is_copy_constructible<pf::polymorphic<int>>::value);
  STATIC_REQUIRE(std::is_move_constructible<pf::polymorphic<int>>::value);
  STATIC_REQUIRE(std::is_copy_assignable<pf::polymorphic<int>>::value);
  STATIC_REQUIRE(std::is_move_assignable<pf::polymorphic<int>>::value);

  STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::polymorphic<int>>::value);

  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::polymorphic<int>&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<const pf::polymorphic<int>&>()), const int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::polymorphic<int>&&>()), int&&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<const pf::polymorphic<int>&&>()), const int&&>::value);
}

TEST_CASE("polymorphic: default construction")
{
  pf::polymorphic<int> p;
  CHECK(!p.valueless_after_move());
  CHECK(*p == 0);
}

TEST_CASE("polymorphic: in_place construction")
{
  pf::polymorphic<int> p(pf::in_place, 42);
  CHECK(*p == 42);
}

TEST_CASE("polymorphic: in_place construction with multiple args")
{
  pf::polymorphic<std::string> s(pf::in_place, 4u, 'z');
  CHECK(*s == "zzzz");
}

TEST_CASE("polymorphic: copy construction deep-copies")
{
  pf::polymorphic<int> a(pf::in_place, 7);
  pf::polymorphic<int> b = a;

  CHECK(*a == 7);
  CHECK(*b == 7);

  *b = 99;
  CHECK(*a == 7);
  CHECK(*b == 99);
}

TEST_CASE("polymorphic: move construction")
{
  pf::polymorphic<int> a(pf::in_place, 7);
  pf::polymorphic<int> b = std::move(a);

  CHECK(*b == 7);
  CHECK(a.valueless_after_move());
}

TEST_CASE("polymorphic: copy assignment")
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b(pf::in_place, 2);
  b = a;

  CHECK(*a == 1);
  CHECK(*b == 1);

  *b = 99;
  CHECK(*a == 1);
}

TEST_CASE("polymorphic: move assignment")
{
  pf::polymorphic<int> a(pf::in_place, 5);
  pf::polymorphic<int> b(pf::in_place, 6);
  b = std::move(a);

  CHECK(*b == 5);
  CHECK(a.valueless_after_move());
}

TEST_CASE("polymorphic: operator->")
{
  pf::polymorphic<std::string> s(pf::in_place, "hello");
  CHECK(s->size() == 5u);
}

TEST_CASE("polymorphic: swap")
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b(pf::in_place, 2);
  a.swap(b);
  CHECK(*a == 2);
  CHECK(*b == 1);

  using std::swap;
  swap(a, b);
  CHECK(*a == 1);
  CHECK(*b == 2);
}

#ifndef YK_POLYFILL_DISABLE_COMPARISON_OPS
TEST_CASE("polymorphic: operator== between two polymorphics")
{
  pf::polymorphic<Point> a(pf::in_place, 1, 2);
  pf::polymorphic<Point> b(pf::in_place, 1, 2);
  pf::polymorphic<Point> c(pf::in_place, 3, 4);

  CHECK(a == b);
  CHECK(!(a == c));
  CHECK(a != c);
}

TEST_CASE("polymorphic: valueless comparison")
{
  pf::polymorphic<int> a(pf::in_place, 1);
  pf::polymorphic<int> b = std::move(a);

  CHECK(a.valueless_after_move());

  pf::polymorphic<int> c = std::move(b);
  CHECK(a == b);
  CHECK(!(a == c));
  CHECK(a != c);
}
#endif  // YK_POLYFILL_DISABLE_COMPARISON_OPS

TEST_CASE("polymorphic: in_place_type_t construction with derived type")
{
  pf::polymorphic<Animal> p(pf::in_place_type_t<Dog>{});
  CHECK(p->sound() == "woof");
}

TEST_CASE("polymorphic: in_place_type_t construction with U == T (class type)")
{
  pf::polymorphic<Point> p(pf::in_place_type_t<Point>{}, 1, 2);
  CHECK(p->x == 1);
  CHECK(p->y == 2);
}

TEST_CASE("polymorphic: self copy-assignment is safe")
{
  pf::polymorphic<int> p(pf::in_place, 42);
  p = p;
  CHECK(*p == 42);
}

TEST_CASE("polymorphic: self move-assignment is safe")
{
  pf::polymorphic<int> p(pf::in_place, 42);
  p = std::move(p);
  CHECK(*p == 42);
}

TEST_CASE("polymorphic: copy preserves dynamic type (the key feature)")
{
  pf::polymorphic<Animal> p1(pf::in_place_type_t<Dog>{});
  pf::polymorphic<Animal> p2 = p1;
  CHECK(p1->sound() == "woof");
  CHECK(p2->sound() == "woof");
}

TEST_CASE("polymorphic: two different derived types are independent")
{
  pf::polymorphic<Animal> dog(pf::in_place_type_t<Dog>{});
  pf::polymorphic<Animal> cat(pf::in_place_type_t<Cat>{});

  CHECK(dog->sound() == "woof");
  CHECK(cat->sound() == "meow");

  pf::polymorphic<Animal> dog2 = dog;
  CHECK(dog2->sound() == "woof");

  dog2 = cat;
  CHECK(dog2->sound() == "meow");

  CHECK(dog->sound() == "woof");
  CHECK(cat->sound() == "meow");
}

TEST_CASE("polymorphic: move of derived type")
{
  pf::polymorphic<Animal> dog(pf::in_place_type_t<Dog>{});
  pf::polymorphic<Animal> moved = std::move(dog);

  CHECK(dog.valueless_after_move());
  CHECK(moved->sound() == "woof");
}

TEST_CASE("polymorphic: get_allocator")
{
  pf::polymorphic<int> p;
  STATIC_REQUIRE(std::is_same<decltype(p.get_allocator()), std::allocator<int>>::value);
}

// ---- allocator-propagation tests ------------------------------------------

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

template <class T> using PocswapAlloc = TestAlloc<T, /*Pocs*/true,  false, false>;
template <class T> using PoccaAlloc   = TestAlloc<T, false, /*Pocca*/true,  false>;
template <class T> using PocmaAlloc   = TestAlloc<T, false, false, /*Pocma*/true>;
template <class T> using StaticAlloc  = TestAlloc<T, false, false, false>;

// --- swap ---

TEST_CASE("polymorphic alloc: swap POCS=false only swaps values")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::polymorphic<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  x.swap(y);
  CHECK(*x == 20);
  CHECK(*y == 10);
  CHECK(x.get_allocator() == a1);  // allocator NOT propagated
  CHECK(y.get_allocator() == a2);
}

TEST_CASE("polymorphic alloc: swap POCS=true swaps values and allocators")
{
  PocswapAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, PocswapAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::polymorphic<int, PocswapAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  x.swap(y);
  CHECK(*x == 20);
  CHECK(*y == 10);
  CHECK(x.get_allocator() == a2);  // allocator WAS propagated
  CHECK(y.get_allocator() == a1);
}

// --- copy assignment ---

TEST_CASE("polymorphic alloc: copy assign POCCA=false leaves allocator unchanged")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::polymorphic<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  y = x;
  CHECK(*y == 10);
  CHECK(y.get_allocator() == a2);  // allocator NOT propagated
}

TEST_CASE("polymorphic alloc: copy assign POCCA=true propagates allocator")
{
  PoccaAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, PoccaAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 10);
  pf::polymorphic<int, PoccaAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 20);
  y = x;
  CHECK(*y == 10);
  CHECK(y.get_allocator() == a1);  // allocator WAS propagated
}

TEST_CASE("polymorphic alloc: copy assign POCCA=true preserves dynamic type")
{
  PoccaAlloc<Animal> a1(1), a2(2);
  pf::polymorphic<Animal, PoccaAlloc<Animal>> dog(std::allocator_arg, a1, pf::in_place_type_t<Dog>{});
  pf::polymorphic<Animal, PoccaAlloc<Animal>> dst(std::allocator_arg, a2, pf::in_place_type_t<Cat>{});
  dst = dog;
  CHECK(dst->sound() == "woof");   // dynamic type (Dog) preserved after clone
  CHECK(dst.get_allocator() == a1);
}

// --- move assignment ---

TEST_CASE("polymorphic alloc: move assign POCMA=true steals both value and allocator")
{
  PocmaAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, PocmaAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::polymorphic<int, PocmaAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(x.valueless_after_move());
  CHECK(y.get_allocator() == a1);  // allocator WAS propagated (stolen)
}

TEST_CASE("polymorphic alloc: move assign POCMA=false same alloc steals value")
{
  StaticAlloc<int> a1(1);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::polymorphic<int, StaticAlloc<int>> y(std::allocator_arg, a1, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(x.valueless_after_move());
  CHECK(y.get_allocator() == a1);
}

TEST_CASE("polymorphic alloc: move assign POCMA=false different alloc clones with own allocator")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 42);
  pf::polymorphic<int, StaticAlloc<int>> y(std::allocator_arg, a2, pf::in_place, 0);
  y = std::move(x);
  CHECK(*y == 42);
  CHECK(y.get_allocator() == a2);  // allocator unchanged
}

TEST_CASE("polymorphic alloc: move assign POCMA=false different alloc preserves dynamic type")
{
  StaticAlloc<Animal> a1(1), a2(2);
  pf::polymorphic<Animal, StaticAlloc<Animal>> src(std::allocator_arg, a1, pf::in_place_type_t<Dog>{});
  pf::polymorphic<Animal, StaticAlloc<Animal>> dst(std::allocator_arg, a2, pf::in_place_type_t<Cat>{});
  dst = std::move(src);
  CHECK(dst->sound() == "woof");  // Dog cloned into dst's allocator
  CHECK(dst.get_allocator() == a2);
}

// --- extended-allocator move constructor ---

TEST_CASE("polymorphic alloc: move ctor with same allocator steals control block")
{
  StaticAlloc<int> a1(1);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 7);
  pf::polymorphic<int, StaticAlloc<int>> y(std::move(x), std::allocator_arg, a1);
  CHECK(*y == 7);
  CHECK(x.valueless_after_move());
}

TEST_CASE("polymorphic alloc: move ctor with different allocator clones")
{
  StaticAlloc<int> a1(1), a2(2);
  pf::polymorphic<int, StaticAlloc<int>> x(std::allocator_arg, a1, pf::in_place, 7);
  pf::polymorphic<int, StaticAlloc<int>> y(std::move(x), std::allocator_arg, a2);
  CHECK(*y == 7);
  CHECK(!x.valueless_after_move());  // x was cloned, not stolen
  CHECK(y.get_allocator() == a2);
}
