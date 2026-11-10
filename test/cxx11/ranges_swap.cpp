#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/concepts.hpp>
#include <yk/polyfill/functional.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

using swap_fn = decltype((pf::ranges::swap));

template<class T, class U>
struct can_swap : pf::is_invocable<swap_fn, T, U> {};

template<class T, class U>
struct can_nothrow_swap : pf::is_nothrow_invocable<swap_fn, T, U> {};

struct has_adl_swap {
  int value;
  bool via_adl;
  explicit has_adl_swap(int v) : value(v), via_adl(false) {}
};

void swap(has_adl_swap& a, has_adl_swap& b)
{
  std::swap(a.value, b.value);
  a.via_adl = true;
  b.via_adl = true;
}

// an ADL swap for Base only; for Derived, the poison pill is the better (exact) match
struct base_with_swap {
  int base;
  bool via_adl;
};

void swap(base_with_swap& a, base_with_swap& b) noexcept
{
  std::swap(a.base, b.base);
  a.via_adl = true;
  b.via_adl = true;
}

struct derived_from_base_with_swap : base_with_swap {
  int derived;
};

struct no_move {
  int value;
  explicit no_move(int v) : value(v) {}
  no_move(no_move&&) = delete;
};

struct no_move_with_swap {
  int value;
  explicit no_move_with_swap(int v) : value(v) {}
  no_move_with_swap(no_move_with_swap&&) = delete;
  friend void swap(no_move_with_swap& a, no_move_with_swap& b) noexcept { std::swap(a.value, b.value); }
};

// movable, but not assignable_from<T&, T>: the assignment must yield exactly T&
struct void_assign {
  int value;
  explicit void_assign(int v) : value(v) {}
  void_assign(void_assign&&) = default;
  void operator=(void_assign&& o) { value = o.value; }
};

struct value_assign {
  int value;
  explicit value_assign(int v) : value(v) {}
  value_assign(value_assign&&) = default;
  value_assign(value_assign const&) = default;
  value_assign operator=(value_assign&& o)
  {
    value = o.value;
    return *this;
  }
};

struct throwing_move {
  int value;
  explicit throwing_move(int v) : value(v) {}
  throwing_move(throwing_move&& o) noexcept(false) : value(o.value) {}
  throwing_move& operator=(throwing_move&& o) noexcept(false)
  {
    value = o.value;
    return *this;
  }
};

struct left {
  int value;
};

struct right {
  int value;
};

void swap(left& l, right& r) noexcept { std::swap(l.value, r.value); }

// ADL reaches these through the pointee and element type, but ranges::swap uses an ADL swap only
// for class or enumeration types, so pointers are exchanged and arrays swapped element-wise
struct adl_bait {
  int value;
  bool baited;
};

void swap(adl_bait*& a, adl_bait*& b)
{
  a->baited = true;
  b->baited = true;
}

void swap(adl_bait (&a)[1], adl_bait (&b)[1])
{
  a[0].baited = true;
  b[0].baited = true;
}

struct incomplete;

// instantiating holder<incomplete> is ill-formed, so nothing may look up swap through holder<incomplete>*
template<class T>
struct holder {
  T value;
};

}  // namespace

namespace unconstrained {

struct widget {
  int value;
  bool via_template;
};

// precluded by the poison pill: ambiguous with it, so ranges::swap falls back to exchange
template<class T>
void swap(T& a, T& b)
{
  a.via_template = true;
  b.via_template = true;
}

}  // namespace unconstrained

TEST_CASE("ranges::swap")
{
  SECTION("exchanges scalars")
  {
    int a = 1, b = 2;
    pf::ranges::swap(a, b);
    CHECK(a == 2);
    CHECK(b == 1);
    STATIC_REQUIRE(can_nothrow_swap<int&, int&>::value);
  }

  SECTION("calls an ADL swap")
  {
    has_adl_swap a(1), b(2);
    pf::ranges::swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    CHECK(a.via_adl);
    STATIC_REQUIRE(can_swap<has_adl_swap&, has_adl_swap&>::value);
    STATIC_REQUIRE_FALSE(can_nothrow_swap<has_adl_swap&, has_adl_swap&>::value);
  }

  SECTION("an ADL swap for a base class loses to the poison pill")
  {
    derived_from_base_with_swap a{}, b{};
    a.base = 1;
    a.derived = 10;
    b.base = 2;
    b.derived = 20;
    pf::ranges::swap(a, b);
    CHECK(a.base == 2);
    CHECK(a.derived == 20);
    CHECK(b.base == 1);
    CHECK(b.derived == 10);
    CHECK_FALSE(a.via_adl);
  }

  SECTION("an unconstrained program-defined swap template is not selected")
  {
    unconstrained::widget a{1, false}, b{2, false};
    pf::ranges::swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    CHECK_FALSE(a.via_template);
  }

  SECTION("swaps std types through their own swap")
  {
    std::string a = "a", b = "b";
    pf::ranges::swap(a, b);
    CHECK(a == "b");
    CHECK(b == "a");
    STATIC_REQUIRE(can_nothrow_swap<std::string&, std::string&>::value);
  }

  SECTION("swaps different types through an ADL swap")
  {
    left l{1};
    right r{2};
    pf::ranges::swap(l, r);
    CHECK(l.value == 2);
    CHECK(r.value == 1);
    STATIC_REQUIRE(can_nothrow_swap<left&, right&>::value);
    STATIC_REQUIRE_FALSE(can_swap<right&, left&>::value);
  }

  SECTION("does not instantiate a pointee class template")
  {
    int marker = 0;
    holder<incomplete>* const target = static_cast<holder<incomplete>*>(static_cast<void*>(&marker));
    holder<incomplete>* p = nullptr;
    holder<incomplete>* q = target;
    pf::ranges::swap(p, q);
    // compared to bool first: Catch2 would look up operator<< for holder<incomplete>* to print it
    bool const swapped = p == target && q == nullptr;
    CHECK(swapped);
    STATIC_REQUIRE(can_nothrow_swap<holder<incomplete>*&, holder<incomplete>*&>::value);
  }

  SECTION("does not use an ADL swap for pointers and arrays")
  {
    adl_bait x{1, false}, y{2, false};
    adl_bait* p = &x;
    adl_bait* q = &y;
    pf::ranges::swap(p, q);
    CHECK(p == &y);
    CHECK(q == &x);
    CHECK_FALSE(x.baited);
    CHECK_FALSE(y.baited);

    adl_bait a[1] = {{1, false}}, b[1] = {{2, false}};
    pf::ranges::swap(a, b);
    CHECK(a[0].value == 2);
    CHECK(b[0].value == 1);
    CHECK_FALSE(a[0].baited);
    CHECK_FALSE(b[0].baited);
  }

  SECTION("swaps arrays element-wise")
  {
    int a[2] = {1, 2}, b[2] = {3, 4};
    pf::ranges::swap(a, b);
    CHECK(a[0] == 3);
    CHECK(a[1] == 4);
    CHECK(b[0] == 1);
    CHECK(b[1] == 2);

    int m[2][2] = {{1, 2}, {3, 4}}, n[2][2] = {{5, 6}, {7, 8}};
    pf::ranges::swap(m, n);
    CHECK(m[0][0] == 5);
    CHECK(m[1][1] == 8);
    CHECK(n[0][0] == 1);
    CHECK(n[1][1] == 4);

    has_adl_swap x[1] = {has_adl_swap(1)}, y[1] = {has_adl_swap(2)};
    pf::ranges::swap(x, y);
    CHECK(x[0].value == 2);
    CHECK(x[0].via_adl);

    STATIC_REQUIRE(can_nothrow_swap<int (&)[2], int (&)[2]>::value);
    STATIC_REQUIRE_FALSE(can_nothrow_swap<has_adl_swap(&)[1], has_adl_swap(&)[1]>::value);
    STATIC_REQUIRE_FALSE(can_swap<int (&)[2], int (&)[3]>::value);
  }

  SECTION("noexcept of exchange follows the move operations")
  {
    throwing_move a(1), b(2);
    pf::ranges::swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    STATIC_REQUIRE(can_swap<throwing_move&, throwing_move&>::value);
    STATIC_REQUIRE_FALSE(can_nothrow_swap<throwing_move&, throwing_move&>::value);
  }

  SECTION("ill-formed without an ADL swap or move operations")
  {
    STATIC_REQUIRE_FALSE(can_swap<no_move&, no_move&>::value);
    STATIC_REQUIRE_FALSE(can_swap<no_move(&)[2], no_move(&)[2]>::value);
    STATIC_REQUIRE_FALSE(can_swap<int const&, int const&>::value);
    STATIC_REQUIRE_FALSE(can_swap<int&&, int&&>::value);
    STATIC_REQUIRE_FALSE(can_swap<void_assign&, void_assign&>::value);
    STATIC_REQUIRE_FALSE(can_swap<value_assign&, value_assign&>::value);

    STATIC_REQUIRE(can_nothrow_swap<no_move_with_swap&, no_move_with_swap&>::value);
    no_move_with_swap a(1), b(2);
    pf::ranges::swap(a, b);
    CHECK(a.value == 2);
  }
}
