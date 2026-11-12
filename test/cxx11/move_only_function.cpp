#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/memory.hpp>
#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
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

struct Adder {
  int sum;
  Adder(int x, int y) : sum(x + y) {}
  int operator()(int z) const { return sum + z; }
};

struct ListSum {
  int sum = 0;
  ListSum(std::initializer_list<int> il, int extra) : sum(extra)
  {
    for (int x : il) sum += x;
  }
  int operator()() const { return sum; }
};

// Callables that return their own address, to observe where the target object is stored.
template<std::size_t Pointers>
struct SelfAddress {
  void* pad[Pointers];
  void const* operator()() const { return this; }
};

struct ThrowingMoveSelfAddress {
  ThrowingMoveSelfAddress() = default;
  ThrowingMoveSelfAddress(ThrowingMoveSelfAddress&&) noexcept(false) {}
  void const* operator()() const { return this; }
};

struct alignas(2 * alignof(void*)) OverAlignedSelfAddress {
  void const* operator()() const { return this; }
};

// Aligned beyond what operator new guarantees without the alignment-aware forms of C++17.
struct alignas(64) ExtendedAlignedSelfAddress {
  void const* operator()() const { return this; }
};

// Observes that the allocation and deallocation of the target match. Too large to be stored in place.
struct ClassAllocated {
  static int allocations;
  static int deallocations;
  void* pad[4] = {};

  static void* operator new(std::size_t size)
  {
    ++allocations;
    return ::operator new(size);
  }

  static void operator delete(void* p) noexcept
  {
    ++deallocations;
    ::operator delete(p);
  }

  void operator()() const {}
};

int ClassAllocated::allocations = 0;
int ClassAllocated::deallocations = 0;

template<class Wrapper>
bool is_within(Wrapper const& w, void const* p)
{
  auto const first = reinterpret_cast<unsigned char const*>(std::addressof(w));
  std::less<void const*> const less;
  return !less(p, first) && less(p, first + sizeof(w));
}

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

TEST_CASE("move_only_function: in_place_type construction")
{
  {
    pf::move_only_function<int(int)> f(pf::in_place_type_t<Adder>{}, 20, 20);
    CHECK(f(2) == 42);
  }
  {
    pf::move_only_function<int()> f(pf::in_place_type_t<ListSum>{}, {10, 20}, 12);
    CHECK(f() == 42);
  }
  {
    // unlike the F&& constructor, a null function pointer still becomes the target
    pf::move_only_function<void()> const f(pf::in_place_type_t<void (*)()>{}, nullptr);
    CHECK(f);
  }

  STATIC_REQUIRE(std::is_constructible<pf::move_only_function<int(int)>, pf::in_place_type_t<Adder>, int, int>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int)>, pf::in_place_type_t<Adder>, int>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int()>, pf::in_place_type_t<Adder>, int, int>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int(int) const>, pf::in_place_type_t<OnlyMutable>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::move_only_function<int()>, pf::in_place_type_t<ListSum>, std::initializer_list<int>, int>::value);
  STATIC_REQUIRE_FALSE(std::is_constructible<pf::move_only_function<int()>, pf::in_place_type_t<ListSum>, std::initializer_list<int>>::value);
  STATIC_REQUIRE_FALSE(std::is_convertible<pf::in_place_type_t<Counter>, pf::move_only_function<int()>>::value);
}

TEST_CASE("move_only_function: assignment")
{
  STATIC_REQUIRE_FALSE(std::is_copy_assignable<pf::move_only_function<void()>>::value);
  STATIC_REQUIRE(std::is_nothrow_assignable<pf::move_only_function<void()>&, std::nullptr_t>::value);
  STATIC_REQUIRE(std::is_assignable<pf::move_only_function<int(int)>&, int (*)(int)>::value);
  STATIC_REQUIRE_FALSE(std::is_assignable<pf::move_only_function<int(int)>&, int>::value);
  STATIC_REQUIRE_FALSE(std::is_assignable<pf::move_only_function<int(int) const>&, OnlyMutable>::value);

  {
    int destroyed = 0;
    pf::move_only_function<void()> f = DestructionProbe(&destroyed);
    pf::move_only_function<void()>& result = (f = nullptr);
    CHECK(&result == &f);
    CHECK_FALSE(f);
    CHECK(destroyed == 1);
  }
  {
    int destroyed_f = 0;
    int destroyed_g = 0;
    {
      pf::move_only_function<void()> f = DestructionProbe(&destroyed_f);
      pf::move_only_function<void()> g = DestructionProbe(&destroyed_g);
      pf::move_only_function<void()>& result = (f = std::move(g));
      CHECK(&result == &f);
      REQUIRE(f);
      CHECK(destroyed_f == 1);
      CHECK(destroyed_g == 0);
    }
    CHECK(destroyed_g == 1);
  }
  {
    // self-move-assignment keeps the target, as it is specified as move-and-swap
    pf::move_only_function<int()> f = Counter{};
    CHECK(f() == 1);
    pf::move_only_function<int()>& alias = f;
    f = std::move(alias);
    REQUIRE(f);
    CHECK(f() == 2);
  }
  {
    pf::move_only_function<int(int)> f;
    pf::move_only_function<int(int)>& result = (f = doubles);
    CHECK(&result == &f);
    CHECK(f(21) == 42);
    f = [](int x) { return x + 1; };
    CHECK(f(41) == 42);
    int (*null_fp)(int) = nullptr;
    f = null_fp;
    CHECK_FALSE(f);
  }
}

TEST_CASE("move_only_function: swap")
{
  STATIC_REQUIRE(noexcept(std::declval<pf::move_only_function<void()>&>().swap(std::declval<pf::move_only_function<void()>&>())));
  STATIC_REQUIRE(pf::is_nothrow_swappable<pf::move_only_function<void()>>::value);

  pf::move_only_function<int()> f = Counter{};
  pf::move_only_function<int()> g = [] { return 42; };
  CHECK(f() == 1);

  f.swap(g);
  CHECK(f() == 42);
  CHECK(g() == 2);

  swap(f, g);
  CHECK(f() == 3);
  CHECK(g() == 42);

  f.swap(f);
  CHECK(f() == 4);

  pf::move_only_function<int()> empty;
  f.swap(empty);
  CHECK_FALSE(f);
  REQUIRE(empty);
  CHECK(empty() == 5);
}

TEST_CASE("move_only_function: comparison with nullptr")
{
  pf::move_only_function<void()> const empty;
  pf::move_only_function<void()> const f = [] {};

  CHECK(empty == nullptr);
  CHECK(nullptr == empty);
  CHECK_FALSE(empty != nullptr);
  CHECK_FALSE(nullptr != empty);
  CHECK_FALSE(f == nullptr);
  CHECK_FALSE(nullptr == f);
  CHECK(f != nullptr);
  CHECK(nullptr != f);

  STATIC_REQUIRE(noexcept(empty == nullptr));
  STATIC_REQUIRE(noexcept(nullptr == empty));
  STATIC_REQUIRE(noexcept(empty != nullptr));
  STATIC_REQUIRE(noexcept(nullptr != empty));
}

TEST_CASE("move_only_function: small-buffer storage")
{
  using wrapper = pf::move_only_function<void const*() const>;
  {
    // up to three pointers with a non-throwing move are stored in place, and moved along with the wrapper
    wrapper f = SelfAddress<3>{};
    CHECK(is_within(f, f()));
    wrapper g(std::move(f));
    CHECK(is_within(g, g()));
    wrapper h;
    h = std::move(g);
    CHECK(is_within(h, h()));
  }
  {
    wrapper const f(pf::in_place_type_t<SelfAddress<1>>{});
    CHECK(is_within(f, f()));
  }
  {
    wrapper const f = SelfAddress<4>{};
    CHECK_FALSE(is_within(f, f()));
  }
  {
    wrapper const f = ThrowingMoveSelfAddress{};
    CHECK_FALSE(is_within(f, f()));
  }
  {
    wrapper const f = OverAlignedSelfAddress{};
    void const* const p = f();
    CHECK_FALSE(is_within(f, p));
    CHECK(reinterpret_cast<std::uintptr_t>(p) % alignof(OverAlignedSelfAddress) == 0);
  }
  {
    // swapping exchanges an in-place target with an allocated one
    wrapper f = SelfAddress<1>{};
    wrapper g = SelfAddress<4>{};
    f.swap(g);
    CHECK_FALSE(is_within(f, f()));
    CHECK(is_within(g, g()));
  }
}

TEST_CASE("move_only_function: allocation and deallocation of the target match")
{
  ClassAllocated::allocations = 0;
  ClassAllocated::deallocations = 0;
  {
    pf::move_only_function<void()> const f = ClassAllocated{};
    CHECK(f);
  }
  CHECK(ClassAllocated::allocations == ClassAllocated::deallocations);
}

TEST_CASE("move_only_function: targets with extended alignment are aligned")
{
  using wrapper = pf::move_only_function<void const*() const>;
  auto const is_aligned = [](void const* p) { return reinterpret_cast<std::uintptr_t>(p) % alignof(ExtendedAlignedSelfAddress) == 0; };

  // several targets, so that an allocation that happens to be aligned does not hide a misalignment
  wrapper fs[8];
  for (wrapper& f : fs) f = ExtendedAlignedSelfAddress{};
  for (wrapper const& f : fs) CHECK(is_aligned(f()));

  wrapper g(pf::in_place_type_t<ExtendedAlignedSelfAddress>{});
  CHECK(is_aligned(g()));
  wrapper h(std::move(g));
  CHECK(is_aligned(h()));
  h.swap(fs[0]);
  CHECK(is_aligned(h()));
  CHECK(is_aligned(fs[0]()));
}
