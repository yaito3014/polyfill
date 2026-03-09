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

  // Move constructor is noexcept
  STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::polymorphic<int>>::value);

  // Dereference value categories
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
  pf::polymorphic<int> b = std::move(a);  // a is now valueless

  CHECK(a.valueless_after_move());

  pf::polymorphic<int> c = std::move(b);  // b is now valueless too
  CHECK(a == b);   // both valueless -> equal
  CHECK(!(a == c));
  CHECK(a != c);
}

TEST_CASE("polymorphic: in_place_type_t construction with derived type")
{
  // Construct a polymorphic<Animal> holding a Dog
  pf::polymorphic<Animal> p(pf::in_place_type<Dog>);
  CHECK(p->sound() == "woof");
}

TEST_CASE("polymorphic: copy preserves dynamic type (the key feature)")
{
  // p1 holds a Dog (derived from Animal)
  pf::polymorphic<Animal> p1(pf::in_place_type<Dog>);
  CHECK(p1->sound() == "woof");

  // Copying p1 should produce a polymorphic<Animal> that still holds a Dog
  pf::polymorphic<Animal> p2 = p1;
  CHECK(p2->sound() == "woof");

  // Mutating p2 should not affect p1 (they are independent objects)
  // We verify independence by checking both still exist
  CHECK(p1->sound() == "woof");
  CHECK(p2->sound() == "woof");
}

TEST_CASE("polymorphic: two different derived types are independent")
{
  pf::polymorphic<Animal> dog(pf::in_place_type<Dog>);
  pf::polymorphic<Animal> cat(pf::in_place_type<Cat>);

  CHECK(dog->sound() == "woof");
  CHECK(cat->sound() == "meow");

  // Copy dog into a new polymorphic
  pf::polymorphic<Animal> dog2 = dog;
  CHECK(dog2->sound() == "woof");

  // Assign cat to dog2: should now hold a Cat
  dog2 = cat;
  CHECK(dog2->sound() == "meow");

  // Original dog and cat are unchanged
  CHECK(dog->sound() == "woof");
  CHECK(cat->sound() == "meow");
}

TEST_CASE("polymorphic: move of derived type")
{
  pf::polymorphic<Animal> dog(pf::in_place_type<Dog>);
  pf::polymorphic<Animal> moved = std::move(dog);

  CHECK(dog.valueless_after_move());
  CHECK(moved->sound() == "woof");
}

TEST_CASE("polymorphic: get_allocator")
{
  pf::polymorphic<int> p;
  STATIC_REQUIRE(std::is_same<decltype(p.get_allocator()), std::allocator<int>>::value);
}
