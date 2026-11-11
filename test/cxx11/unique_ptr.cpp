#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/memory.hpp>

#include <functional>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace pf = yk::polyfill;

namespace {

struct Base {
  virtual ~Base() = default;
  virtual int id() const { return 0; }
};

struct Derived : Base {
  int id() const override { return 1; }
};

struct custom_deleter {
  bool* deleted;
  explicit custom_deleter(bool* d) : deleted(d) {}
  void operator()(int* p) const
  {
    *deleted = true;
    delete p;
  }
};

int g_swap_count = 0;

struct swap_tracking_deleter {
  int tag;
  explicit swap_tracking_deleter(int t = 0) : tag(t) {}
  void operator()(int* p) const { delete p; }
};

void swap(swap_tracking_deleter& a, swap_tracking_deleter& b) noexcept
{
  int tmp = a.tag;
  a.tag = b.tag;
  b.tag = tmp;
  ++g_swap_count;
}

struct array_swap_tracking_deleter {
  int tag;
  explicit array_swap_tracking_deleter(int t = 0) : tag(t) {}
  void operator()(int* p) const { delete[] p; }
};

void swap(array_swap_tracking_deleter& a, array_swap_tracking_deleter& b) noexcept
{
  int tmp = a.tag;
  a.tag = b.tag;
  b.tag = tmp;
  ++g_swap_count;
}

#if __cplusplus < 202002L
// A non-owning fancy pointer swapped through an explicit specialization of std::swap.
struct specialized_pointer {
  int* p;
  bool via_specialization;

  specialized_pointer() : p(nullptr), via_specialization(false) {}
  specialized_pointer(std::nullptr_t) : p(nullptr), via_specialization(false) {}
  explicit specialized_pointer(int* q) : p(q), via_specialization(false) {}

  explicit operator bool() const { return p != nullptr; }
  friend bool operator==(specialized_pointer const& a, specialized_pointer const& b) { return a.p == b.p; }
  friend bool operator!=(specialized_pointer const& a, specialized_pointer const& b) { return a.p != b.p; }
};

struct specialized_pointer_deleter {
  using pointer = specialized_pointer;
  void operator()(pointer) const noexcept {}
};
#endif

#if __cplusplus > 202002L
// The std::swap context finds this through ADL on the pointee, and prefers it to std::swap.
struct adl_pointee {
  bool via_pointer_swap;
};

void swap(adl_pointee*& a, adl_pointee*& b) noexcept
{
  adl_pointee* const tmp = a;
  a = b;
  b = tmp;
  a->via_pointer_swap = true;
  b->via_pointer_swap = true;
}

struct noop_deleter {
  template<class P>
  void operator()(P) const noexcept
  {
  }
};
#endif

struct custom_pointer_deleter {
  using pointer = long*;
  void operator()(pointer p) const { delete p; }
};

struct multi_arg {
  int x;
  std::string s;
  multi_arg(int x, std::string const& s) : x(x), s(s) {}
};

}  // namespace

#if __cplusplus < 202002L
// Allowed before C++20 ([namespace.std]); since C++20 adding it to namespace std is undefined.
namespace std {

template<>
void swap<specialized_pointer>(specialized_pointer& a, specialized_pointer& b) noexcept
{
  int* const tmp = a.p;
  a.p = b.p;
  b.p = tmp;
  a.via_specialization = true;
  b.via_specialization = true;
}

}  // namespace std
#endif

TEST_CASE("unique_ptr - basic")
{
  SECTION("default constructor")
  {
    pf::unique_ptr<int> p;
    CHECK(!p);
    CHECK(p.get() == nullptr);
  }

  SECTION("nullptr constructor")
  {
    pf::unique_ptr<int> p(nullptr);
    CHECK(!p);
  }

  SECTION("pointer constructor")
  {
    pf::unique_ptr<int> p(new int(42));
    REQUIRE(p);
    CHECK(*p == 42);
  }

  SECTION("dereference and arrow")
  {
    pf::unique_ptr<Base> p(new Derived);
    CHECK(p->id() == 1);
    CHECK((*p).id() == 1);
  }
}

TEST_CASE("unique_ptr - move semantics")
{
  SECTION("move constructor")
  {
    pf::unique_ptr<int> a(new int(10));
    int* raw = a.get();
    pf::unique_ptr<int> b(static_cast<pf::unique_ptr<int>&&>(a));
    CHECK(!a);
    CHECK(b.get() == raw);
    CHECK(*b == 10);
  }

  SECTION("move assignment")
  {
    pf::unique_ptr<int> a(new int(10));
    pf::unique_ptr<int> b(new int(20));
    int* raw = a.get();
    b = static_cast<pf::unique_ptr<int>&&>(a);
    CHECK(!a);
    CHECK(b.get() == raw);
    CHECK(*b == 10);
  }

  SECTION("converting move constructor")
  {
    pf::unique_ptr<Derived> d(new Derived);
    pf::unique_ptr<Base> b(static_cast<pf::unique_ptr<Derived>&&>(d));
    CHECK(!d);
    REQUIRE(b);
    CHECK(b->id() == 1);
  }

  SECTION("nullptr assignment")
  {
    pf::unique_ptr<int> p(new int(42));
    p = nullptr;
    CHECK(!p);
  }
}

TEST_CASE("unique_ptr - modifiers")
{
  SECTION("release")
  {
    pf::unique_ptr<int> p(new int(42));
    int* raw = p.release();
    CHECK(!p);
    CHECK(*raw == 42);
    delete raw;
  }

  SECTION("reset")
  {
    pf::unique_ptr<int> p(new int(42));
    p.reset(new int(99));
    CHECK(*p == 99);
  }

  SECTION("reset to null")
  {
    pf::unique_ptr<int> p(new int(42));
    p.reset();
    CHECK(!p);
  }

  SECTION("swap")
  {
    pf::unique_ptr<int> a(new int(1));
    pf::unique_ptr<int> b(new int(2));
    int* ra = a.get();
    int* rb = b.get();
    a.swap(b);
    CHECK(a.get() == rb);
    CHECK(b.get() == ra);
    CHECK(*a == 2);
    CHECK(*b == 1);
  }
}

TEST_CASE("unique_ptr - custom deleter")
{
  SECTION("deleter is called")
  {
    bool deleted = false;
    {
      pf::unique_ptr<int, custom_deleter> p(new int(1), custom_deleter(&deleted));
      CHECK(!deleted);
    }
    CHECK(deleted);
  }

  SECTION("reference deleter")
  {
    bool deleted = false;
    custom_deleter d(&deleted);
    {
      pf::unique_ptr<int, custom_deleter&> p(new int(1), d);
      CHECK(&p.get_deleter() == &d);
    }
    CHECK(deleted);
  }
}

TEST_CASE("unique_ptr - custom pointer type in deleter")
{
  pf::unique_ptr<int, custom_pointer_deleter> p(new long(42));
  STATIC_REQUIRE(std::is_same<decltype(p)::pointer, long*>::value);
  REQUIRE(p);
  CHECK(*p.get() == 42);

  long* raw = p.release();
  CHECK(!p);
  delete raw;
}

TEST_CASE("unique_ptr - swap calls ADL swap on deleter")
{
  g_swap_count = 0;

  pf::unique_ptr<int, swap_tracking_deleter> a(new int(1), swap_tracking_deleter(10));
  pf::unique_ptr<int, swap_tracking_deleter> b(new int(2), swap_tracking_deleter(20));

  a.swap(b);

  CHECK(*a == 2);
  CHECK(*b == 1);
  CHECK(a.get_deleter().tag == 20);
  CHECK(b.get_deleter().tag == 10);
  CHECK(g_swap_count == 1);
}

#if __cplusplus < 202002L
// Before C++23, [unique.ptr.single.modifiers] "invokes swap" on the stored pointers means
// ::std::swap ([contents]), which uses its explicit specializations (allowed before C++20).
TEST_CASE("unique_ptr - swap calls an explicit specialization of std::swap on the pointer")
{
  int x = 1, y = 2;

  pf::unique_ptr<int, specialized_pointer_deleter> a(specialized_pointer{&x}), b(specialized_pointer{&y});
  a.swap(b);
  CHECK(a.get().p == &y);
  CHECK(b.get().p == &x);
  CHECK(a.get().via_specialization);

  pf::unique_ptr<int[], specialized_pointer_deleter> c(specialized_pointer{&x}), d(specialized_pointer{&y});
  c.swap(d);
  CHECK(c.get().p == &y);
  CHECK(d.get().p == &x);
  CHECK(c.get().via_specialization);
}
#endif

#if __cplusplus > 202002L
// Since C++23, [contents] gives the unqualified swap the meaning of [swappable.requirements].
TEST_CASE("unique_ptr - swap calls an ADL swap for a raw pointer")
{
  adl_pointee x{false}, y{false};

  pf::unique_ptr<adl_pointee, noop_deleter> a(&x), b(&y);
  a.swap(b);
  CHECK(a.get() == &y);
  CHECK(b.get() == &x);
  CHECK(x.via_pointer_swap);

  x.via_pointer_swap = false;
  pf::unique_ptr<adl_pointee[], noop_deleter> c(&x), d(&y);
  c.swap(d);
  CHECK(c.get() == &y);
  CHECK(d.get() == &x);
  CHECK(x.via_pointer_swap);
}
#endif

TEST_CASE("unique_ptr - comparisons")
{
  SECTION("nullptr comparisons")
  {
    pf::unique_ptr<int> null;
    pf::unique_ptr<int> non_null(new int(1));

    CHECK(null == nullptr);
    CHECK(nullptr == null);
    CHECK(!(null != nullptr));
    CHECK(non_null != nullptr);
    CHECK(nullptr != non_null);
    CHECK(!(non_null == nullptr));
  }

  SECTION("pointer comparisons")
  {
    pf::unique_ptr<int> a(new int(1));
    pf::unique_ptr<int> b(new int(2));

    CHECK(!(a == b));
    CHECK(a != b);

    pf::unique_ptr<int> c;
    pf::unique_ptr<int> d;
    CHECK(c == d);
  }

  SECTION("ordering - self consistency")
  {
    pf::unique_ptr<int> null;

    CHECK(!(null < null));
    CHECK(!(null > null));
    CHECK(null <= null);
    CHECK(null >= null);
  }

  SECTION("ordering - two distinct pointers are consistent")
  {
    pf::unique_ptr<int> a(new int(1));
    pf::unique_ptr<int> b(new int(2));

    bool a_less = a < b;
    CHECK((a < b) == a_less);
    CHECK((b < a) == !a_less);
    CHECK((a > b) == !a_less);
    CHECK((b > a) == a_less);
    CHECK((a <= b) == a_less);
    CHECK((b <= a) == !a_less);
    CHECK((a >= b) == !a_less);
    CHECK((b >= a) == a_less);
  }

  SECTION("ordering - null unique_ptr vs nullptr literal")
  {
    pf::unique_ptr<int> null;

    CHECK(!(null < nullptr));
    CHECK(!(nullptr < null));
    CHECK(!(null > nullptr));
    CHECK(!(nullptr > null));
    CHECK(null <= nullptr);
    CHECK(nullptr <= null);
    CHECK(null >= nullptr);
    CHECK(nullptr >= null);
  }

  SECTION("ordering - non-null unique_ptr vs nullptr literal is consistent")
  {
    pf::unique_ptr<int> p(new int(1));

    bool p_less = p < nullptr;
    CHECK((p < nullptr) == p_less);
    CHECK((nullptr < p) == !p_less);
    CHECK((p > nullptr) == !p_less);
    CHECK((nullptr > p) == p_less);
    CHECK((p <= nullptr) == p_less);
    CHECK((nullptr <= p) == !p_less);
    CHECK((p >= nullptr) == !p_less);
    CHECK((nullptr >= p) == p_less);

    CHECK(p != nullptr);
  }

  SECTION("ordering - non-null unique_ptr vs null unique_ptr is consistent")
  {
    pf::unique_ptr<int> null;
    pf::unique_ptr<int> p(new int(1));

    bool p_less = p < null;
    CHECK((p < null) == p_less);
    CHECK((null < p) == !p_less);
    CHECK((p > null) == !p_less);
    CHECK((null > p) == p_less);
    CHECK((p <= null) == p_less);
    CHECK((null <= p) == !p_less);
    CHECK((p >= null) == !p_less);
    CHECK((null >= p) == p_less);
  }
}

TEST_CASE("unique_ptr - non-member swap")
{
  pf::unique_ptr<int> a(new int(1));
  pf::unique_ptr<int> b(new int(2));
  int* ra = a.get();
  int* rb = b.get();

  pf::swap(a, b);

  CHECK(a.get() == rb);
  CHECK(b.get() == ra);
}

TEST_CASE("unique_ptr - make_unique with multiple args")
{
  auto p = pf::make_unique<multi_arg>(42, "hello");
  REQUIRE(p);
  CHECK(p->x == 42);
  CHECK(p->s == "hello");
}

TEST_CASE("unique_ptr - array specialization")
{
  SECTION("basic")
  {
    pf::unique_ptr<int[]> p(new int[3]);
    REQUIRE(p);
    p[0] = 10;
    p[1] = 20;
    p[2] = 30;
    CHECK(p[0] == 10);
    CHECK(p[1] == 20);
    CHECK(p[2] == 30);
  }

  SECTION("move")
  {
    pf::unique_ptr<int[]> a(new int[2]);
    a[0] = 42;
    pf::unique_ptr<int[]> b(static_cast<pf::unique_ptr<int[]>&&>(a));
    CHECK(!a);
    CHECK(b[0] == 42);
  }

  SECTION("reset")
  {
    pf::unique_ptr<int[]> p(new int[2]);
    p.reset(new int[3]);
    REQUIRE(p);
    p.reset();
    CHECK(!p);
  }

  SECTION("swap calls ADL swap on deleter")
  {
    g_swap_count = 0;

    pf::unique_ptr<int[], array_swap_tracking_deleter> a(new int[1], array_swap_tracking_deleter(10));
    pf::unique_ptr<int[], array_swap_tracking_deleter> b(new int[1], array_swap_tracking_deleter(20));

    a.swap(b);

    CHECK(a.get_deleter().tag == 20);
    CHECK(b.get_deleter().tag == 10);
    CHECK(g_swap_count == 1);
  }

  SECTION("make_unique array")
  {
    auto p = pf::make_unique<int[]>(5);
    REQUIRE(p);
    p[0] = 100;
    CHECK(p[0] == 100);
  }
}

TEST_CASE("unique_ptr - hash")
{
  SECTION("null unique_ptr hashes to hash of nullptr")
  {
    pf::unique_ptr<int> p;
    std::size_t h = std::hash<pf::unique_ptr<int>>{}(p);
    CHECK(h == std::hash<int*>{}(nullptr));
  }

  SECTION("non-null unique_ptr hashes to hash of raw pointer")
  {
    pf::unique_ptr<int> p(new int(42));
    int* raw = p.get();
    std::size_t h = std::hash<pf::unique_ptr<int>>{}(p);
    CHECK(h == std::hash<int*>{}(raw));
  }

  SECTION("usable as unordered_set key")
  {
    std::unordered_set<pf::unique_ptr<int>> s;
    s.insert(pf::make_unique<int>(1));
    s.insert(pf::make_unique<int>(2));
    CHECK(s.size() == 2);
  }

  SECTION("array specialization")
  {
    pf::unique_ptr<int[]> p(new int[3]);
    int* raw = p.get();
    std::size_t h = std::hash<pf::unique_ptr<int[]>>{}(p);
    CHECK(h == std::hash<int*>{}(raw));
  }
}

TEST_CASE("unique_ptr - SFINAE")
{
  STATIC_REQUIRE(!std::is_constructible<pf::unique_ptr<Base[]>, Derived*>::value);
  STATIC_REQUIRE(!std::is_copy_constructible<pf::unique_ptr<int>>::value);
  STATIC_REQUIRE(!std::is_copy_assignable<pf::unique_ptr<int>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::unique_ptr<Base>, pf::unique_ptr<Derived>&&>::value);
}
