#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/memory.hpp>

#include <string>
#include <type_traits>

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

TEST_CASE("unique_ptr - basic")
{
  SECTION("default constructor")
  {
    pf::unique_ptr<int> p;
    CHECK_FALSE(p);
    CHECK(p.get() == nullptr);
  }

  SECTION("nullptr constructor")
  {
    pf::unique_ptr<int> p(nullptr);
    CHECK_FALSE(p);
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
    CHECK_FALSE(a);
    CHECK(b.get() == raw);
    CHECK(*b == 10);
  }

  SECTION("move assignment")
  {
    pf::unique_ptr<int> a(new int(10));
    pf::unique_ptr<int> b(new int(20));
    int* raw = a.get();
    b = static_cast<pf::unique_ptr<int>&&>(a);
    CHECK_FALSE(a);
    CHECK(b.get() == raw);
    CHECK(*b == 10);
  }

  SECTION("converting move constructor")
  {
    pf::unique_ptr<Derived> d(new Derived);
    pf::unique_ptr<Base> b(static_cast<pf::unique_ptr<Derived>&&>(d));
    CHECK_FALSE(d);
    REQUIRE(b);
    CHECK(b->id() == 1);
  }

  SECTION("nullptr assignment")
  {
    pf::unique_ptr<int> p(new int(42));
    p = nullptr;
    CHECK_FALSE(p);
  }
}

TEST_CASE("unique_ptr - modifiers")
{
  SECTION("release")
  {
    pf::unique_ptr<int> p(new int(42));
    int* raw = p.release();
    CHECK_FALSE(p);
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
    CHECK_FALSE(p);
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
      CHECK_FALSE(deleted);
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
  CHECK_FALSE(p);
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

TEST_CASE("unique_ptr - comparisons")
{
  SECTION("nullptr comparisons")
  {
    pf::unique_ptr<int> null;
    pf::unique_ptr<int> non_null(new int(1));

    CHECK(null == nullptr);
    CHECK(nullptr == null);
    CHECK_FALSE(null != nullptr);
    CHECK(non_null != nullptr);
    CHECK(nullptr != non_null);
    CHECK_FALSE(non_null == nullptr);
  }

  SECTION("pointer comparisons")
  {
    pf::unique_ptr<int> a(new int(1));
    pf::unique_ptr<int> b(new int(2));

    CHECK_FALSE(a == b);
    CHECK(a != b);

    pf::unique_ptr<int> c;
    pf::unique_ptr<int> d;
    CHECK(c == d);
  }

  SECTION("ordering operators")
  {
    pf::unique_ptr<int> null;
    pf::unique_ptr<int> p(new int(1));

    // null < non-null (on most platforms, nullptr < any valid pointer)
    CHECK(null < p);
    CHECK(null <= p);
    CHECK(p > null);
    CHECK(p >= null);
    CHECK_FALSE(p < null);
    CHECK_FALSE(null > p);

    // self-equality for ordering
    CHECK(null <= null);
    CHECK(null >= null);
    CHECK_FALSE(null < null);
    CHECK_FALSE(null > null);
  }

  SECTION("nullptr ordering operators")
  {
    pf::unique_ptr<int> null;
    pf::unique_ptr<int> p(new int(1));

    CHECK_FALSE(null < nullptr);
    CHECK_FALSE(nullptr < null);
    CHECK(null <= nullptr);
    CHECK(nullptr <= null);
    CHECK(null >= nullptr);
    CHECK(nullptr >= null);

    CHECK(nullptr < p);
    CHECK(p > nullptr);
    CHECK(nullptr <= p);
    CHECK(p >= nullptr);
    CHECK_FALSE(p < nullptr);
    CHECK_FALSE(nullptr > p);
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
    CHECK_FALSE(a);
    CHECK(b[0] == 42);
  }

  SECTION("reset")
  {
    pf::unique_ptr<int[]> p(new int[2]);
    p.reset(new int[3]);
    REQUIRE(p);
    p.reset();
    CHECK_FALSE(p);
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

// SFINAE checks
TEST_CASE("unique_ptr - SFINAE")
{
  // array unique_ptr should not accept pointer-to-derived
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::unique_ptr<Base[]>, Derived*>::value);

  // not copyable
  STATIC_REQUIRE_FALSE(std::is_copy_constructible<pf::unique_ptr<int>>::value);
  STATIC_REQUIRE_FALSE(std::is_copy_assignable<pf::unique_ptr<int>>::value);

  // convertible: Derived* -> Base*
  STATIC_REQUIRE(std::is_constructible<pf::unique_ptr<Base>, pf::unique_ptr<Derived>&&>::value);

}
