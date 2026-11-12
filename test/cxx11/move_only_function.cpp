#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/memory.hpp>
#include <yk/polyfill/utility.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) { return 2 * x; }

struct DifferentForConstness {
  int operator()(int x) const { return 2 * x; }
  int operator()(int x) { return 3 * x; }
};

struct OnlyMutable {
  int operator()(int x) { return 2 * x; }
};

struct OnlyLvalue {
  int operator()(int x) & { return 2 * x; }
};

struct OnlyRvalue {
  int operator()(int x) && { return 3 * x; }
};

struct DifferentForValueCategory {
  int operator()() & { return 1; }
  int operator()() const& { return 2; }
  int operator()() && { return 3; }
  int operator()() const&& { return 4; }
};

struct MoveOnly {
  pf::unique_ptr<int> p;
  explicit MoveOnly(int x) : p(pf::make_unique<int>(x)) {}
  MoveOnly(MoveOnly&&) = default;
  int operator()() const { return *p; }
};

struct Counter {
  int calls = 0;
  int operator()() { return ++calls; }
};

struct DestructionProbe {
  int* destroyed;
  explicit DestructionProbe(int* d) : destroyed(d) {}
  DestructionProbe(DestructionProbe&& other) noexcept : destroyed(other.destroyed) { other.destroyed = nullptr; }
  ~DestructionProbe()
  {
    if (destroyed) ++*destroyed;
  }
  void operator()() const {}
};

struct NotMoveConstructible {
  NotMoveConstructible() = default;
  NotMoveConstructible(NotMoveConstructible&&) = delete;
  void operator()() const {}
};

struct S {
  int val = 7;
  int mem(int x) const { return val * x; }
};

}  // namespace

TEST_CASE("move_only_function: empty states")
{
  STATIC_REQUIRE(std::is_same<pf::move_only_function<int(long)>::result_type, int>::value);

  {
    pf::move_only_function<int(int)> const f;
    CHECK_FALSE(f);
  }
  {
    pf::move_only_function<int(int)> const f(nullptr);
    CHECK_FALSE(f);
  }
  {
    int (*null_fp)(int) = nullptr;
    pf::move_only_function<int(int)> const f(null_fp);
    CHECK_FALSE(f);
  }
  {
    int (S::*null_mp)(int) const = nullptr;
    pf::move_only_function<int(S const&, int)> const f(null_mp);
    CHECK_FALSE(f);
  }
  {
    int S::* null_dmp = nullptr;
    pf::move_only_function<int(S const&)> const f(null_dmp);
    CHECK_FALSE(f);
  }
  {
    // an empty move_only_function of another signature also yields no target
    pf::move_only_function<int(int) const> empty;
    pf::move_only_function<int(int)> const f(std::move(empty));
    CHECK_FALSE(f);
  }

  STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::move_only_function<void()>>::value);
  STATIC_REQUIRE(std::is_nothrow_constructible<pf::move_only_function<void()>, std::nullptr_t>::value);
  STATIC_REQUIRE_FALSE(std::is_convertible<pf::move_only_function<void()>, bool>::value);
}

TEST_CASE("move_only_function: targets")
{
  {
    pf::move_only_function<int(int)> f = doubles;
    REQUIRE(f);
    CHECK(f(21) == 42);
  }
  {
    pf::move_only_function<int(int)> f = &doubles;
    REQUIRE(f);
    CHECK(f(21) == 42);
  }
  {
    pf::move_only_function<int(int)> f = [](int x) { return x + 1; };
    CHECK(f(41) == 42);
  }
  {
    int offset = 40;
    pf::move_only_function<int(int)> f = [offset](int x) { return x + offset; };
    CHECK(f(2) == 42);
  }
  {
    pf::move_only_function<int()> f = MoveOnly(42);
    CHECK(f() == 42);
  }
  {
    pf::move_only_function<int(S const&, int)> f = &S::mem;
    S s;
    CHECK(f(s, 6) == 42);
  }
  {
    pf::move_only_function<int(S const&)> f = &S::val;
    S s;
    CHECK(f(s) == 7);
  }
  {
    // the target is owned: its state persists across calls
    pf::move_only_function<int()> f = Counter{};
    CHECK(f() == 1);
    CHECK(f() == 2);
  }
  {
    // reference_wrapper refers to the original object
    Counter c;
    pf::move_only_function<int()> f = std::ref(c);
    CHECK(f() == 1);
    CHECK(c.calls == 1);
  }
  {
    // the return value converts to R, and is discarded for void
    pf::move_only_function<long(int)> to_long = doubles;
    CHECK(to_long(21) == 42L);
    pf::move_only_function<void(int)> to_void = doubles;
    to_void(21);
  }
  {
    // a non-empty move_only_function of another signature becomes the target
    pf::move_only_function<int(int) const> inner = doubles;
    pf::move_only_function<int(int)> f(std::move(inner));
    REQUIRE(f);
    CHECK(f(21) == 42);
  }
}

TEST_CASE("move_only_function: move and destruction")
{
  STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::move_only_function<void()>>::value);
  STATIC_REQUIRE_FALSE(std::is_copy_constructible<pf::move_only_function<void()>>::value);

  {
    pf::move_only_function<int()> f = MoveOnly(42);
    pf::move_only_function<int()> g(std::move(f));
    REQUIRE(g);
    CHECK(g() == 42);
  }
  {
    int destroyed = 0;
    {
      pf::move_only_function<void()> f = DestructionProbe(&destroyed);
      CHECK(destroyed == 0);
      pf::move_only_function<void()> g(std::move(f));
      CHECK(destroyed == 0);
    }
    CHECK(destroyed == 1);
  }
}

TEST_CASE("move_only_function: constraints")
{
  // F must be callable through the qualifiers of the signature (is-callable-from)
  STATIC_REQUIRE(std::is_constructible<pf::move_only_function<int(int)>, OnlyMutable>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int) const>, OnlyMutable>::value);
  STATIC_REQUIRE(std::is_constructible<pf::move_only_function<int(int) &>, OnlyLvalue>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int)>, OnlyRvalue>::value);
  STATIC_REQUIRE(std::is_constructible<pf::move_only_function<int(int) &&>, OnlyRvalue>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int) &&>, OnlyLvalue>::value);

  // with no ref-qualifier, the target must also be callable as an rvalue
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int)>, OnlyLvalue>::value);

  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int)>, int>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int)>, int (*)(int, int)>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int*()>, int (*)()>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<void()>, pf::in_place_type_t<Counter>>::value);

  // is-callable-from holds but VT is not constructible from F: ill-formed (Mandates), not a constraint
  STATIC_REQUIRE(std::is_convertible<NotMoveConstructible, pf::move_only_function<void()>>::value);
}

TEST_CASE("move_only_function: invocation qualifiers")
{
  {
    pf::move_only_function<int(int)> f = DifferentForConstness{};
    CHECK(f(1) == 3);
  }
  {
    pf::move_only_function<int(int) const> const f = DifferentForConstness{};
    CHECK(f(1) == 2);
  }
  {
    pf::move_only_function<int()> f = DifferentForValueCategory{};
    CHECK(f() == 1);
  }
  {
    pf::move_only_function<int() const> const f = DifferentForValueCategory{};
    CHECK(f() == 2);
  }
  {
    pf::move_only_function<int() &> f = DifferentForValueCategory{};
    CHECK(f() == 1);
  }
  {
    pf::move_only_function<int() const&> const f = DifferentForValueCategory{};
    CHECK(f() == 2);
  }
  {
    pf::move_only_function<int() &&> f = DifferentForValueCategory{};
    CHECK(std::move(f)() == 3);
  }
  {
    pf::move_only_function<int() const&&> const f = DifferentForValueCategory{};
    CHECK(std::move(f)() == 4);
  }
  {
    pf::move_only_function<int(int) &&> f = OnlyRvalue{};
    CHECK(std::move(f)(14) == 42);
  }

  // operator() carries the signature's qualifiers
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void()>&>::value);
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void()>&&>::value);
  STATIC_REQUIRE_FALSE(pf::is_invocable<pf::move_only_function<void()> const&>::value);
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void() const> const&>::value);
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void() &>&>::value);
  STATIC_REQUIRE_FALSE(pf::is_invocable<pf::move_only_function<void() &>&&>::value);
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void() &&>&&>::value);
  STATIC_REQUIRE_FALSE(pf::is_invocable<pf::move_only_function<void() &&>&>::value);
  STATIC_REQUIRE(pf::is_invocable<pf::move_only_function<void() const&&> const&&>::value);
  STATIC_REQUIRE_FALSE(pf::is_invocable<pf::move_only_function<void() const&&> const&>::value);
}

TEST_CASE("move_only_function: arguments are forwarded")
{
  pf::move_only_function<int(pf::unique_ptr<int>)> f = [](pf::unique_ptr<int> p) { return *p; };
  CHECK(f(pf::make_unique<int>(42)) == 42);

  pf::move_only_function<int&(int&)> g = [](int& x) -> int& { return x; };
  int x = 0;
  CHECK(&g(x) == &x);
}
