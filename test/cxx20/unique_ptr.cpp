#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/memory.hpp>

namespace pf = yk::polyfill;

constexpr bool test_make_unique()
{
  auto p = pf::make_unique<int>(42);
  return *p == 42;
}

constexpr bool test_unique_ptr_move()
{
  auto a = pf::make_unique<int>(10);
  auto b = static_cast<pf::unique_ptr<int>&&>(a);
  return !a && *b == 10;
}

constexpr bool test_unique_ptr_reset()
{
  auto p = pf::make_unique<int>(1);
  p.reset(new int(2));
  bool ok = *p == 2;
  p.reset();
  return ok && !p;
}

constexpr bool test_unique_ptr_release()
{
  auto p = pf::make_unique<int>(99);
  int* raw = p.release();
  bool ok = !p && *raw == 99;
  delete raw;
  return ok;
}

constexpr bool test_unique_ptr_swap()
{
  auto a = pf::make_unique<int>(1);
  auto b = pf::make_unique<int>(2);
  a.swap(b);
  return *a == 2 && *b == 1;
}

constexpr bool test_unique_ptr_move_assign()
{
  auto a = pf::make_unique<int>(10);
  auto b = pf::make_unique<int>(20);
  b = static_cast<pf::unique_ptr<int>&&>(a);
  return !a && *b == 10;
}

constexpr bool test_unique_ptr_nullptr_assign()
{
  auto p = pf::make_unique<int>(42);
  p = nullptr;
  return !p;
}

constexpr bool test_unique_ptr_comparisons()
{
  auto p = pf::make_unique<int>(1);
  pf::unique_ptr<int> null;
  return p != null && null == nullptr && p != nullptr;
}

constexpr bool test_make_unique_array()
{
  auto p = pf::make_unique<int[]>(3);
  p[0] = 10;
  p[1] = 20;
  p[2] = 30;
  return p[0] == 10 && p[1] == 20 && p[2] == 30;
}

// Reading a default-initialized element would not be a constant expression.
constexpr bool test_make_unique_array_value_initializes()
{
  auto p = pf::make_unique<int[]>(3);
  return p[0] == 0 && p[1] == 0 && p[2] == 0;
}

struct AggregateElement {
  int v;
};

struct UserConstructedElement {
  int v;
  constexpr UserConstructedElement() : v(0) {}
};

struct ExplicitlyConstructedElement {
  int v;
  explicit constexpr ExplicitlyConstructedElement() : v(0) {}
};

struct OwningElement {
  int* v;
  constexpr OwningElement() : v(new int(0)) {}
  OwningElement(OwningElement const&) = delete;
  OwningElement& operator=(OwningElement const&) = delete;
  constexpr ~OwningElement() { delete v; }
};

template<class E>
constexpr bool test_make_unique_array_value_initializes_class()
{
  auto p = pf::make_unique<E[]>(3);
  return p[0].v == 0 && p[1].v == 0 && p[2].v == 0;
}

constexpr bool test_make_unique_array_value_initializes_owning()
{
  auto p = pf::make_unique<OwningElement[]>(3);
  return *p[0].v == 0 && *p[1].v == 0 && *p[2].v == 0;
}

constexpr bool test_make_unique_array_value_initializes_nested_arrays()
{
  auto p = pf::make_unique<int[][2]>(3);
  return p[0][0] == 0 && p[0][1] == 0 && p[2][0] == 0 && p[2][1] == 0;
}

TEST_CASE("unique_ptr - constexpr")
{
  SECTION("make_unique") { STATIC_REQUIRE(test_make_unique()); }
  SECTION("move") { STATIC_REQUIRE(test_unique_ptr_move()); }
  SECTION("reset") { STATIC_REQUIRE(test_unique_ptr_reset()); }
  SECTION("release") { STATIC_REQUIRE(test_unique_ptr_release()); }
  SECTION("swap") { STATIC_REQUIRE(test_unique_ptr_swap()); }
  SECTION("move assign") { STATIC_REQUIRE(test_unique_ptr_move_assign()); }
  SECTION("nullptr assign") { STATIC_REQUIRE(test_unique_ptr_nullptr_assign()); }
  SECTION("comparisons") { STATIC_REQUIRE(test_unique_ptr_comparisons()); }
  SECTION("array") { STATIC_REQUIRE(test_make_unique_array()); }
  SECTION("array value-initialization") { STATIC_REQUIRE(test_make_unique_array_value_initializes()); }
  SECTION("array value-initialization of an aggregate") { STATIC_REQUIRE(test_make_unique_array_value_initializes_class<AggregateElement>()); }
  SECTION("array value-initialization with a user-provided constructor")
  {
    STATIC_REQUIRE(test_make_unique_array_value_initializes_class<UserConstructedElement>());
  }
  SECTION("array value-initialization with an explicit constructor")
  {
    STATIC_REQUIRE(test_make_unique_array_value_initializes_class<ExplicitlyConstructedElement>());
  }
  SECTION("array value-initialization with a non-trivial destructor") { STATIC_REQUIRE(test_make_unique_array_value_initializes_owning()); }
  SECTION("array value-initialization of nested arrays") { STATIC_REQUIRE(test_make_unique_array_value_initializes_nested_arrays()); }
}
