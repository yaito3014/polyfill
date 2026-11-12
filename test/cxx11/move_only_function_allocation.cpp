// Built as its own executable, since it replaces the global allocation functions to count allocations.

#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <new>
#include <utility>

namespace {

std::size_t allocation_count = 0;
std::size_t deallocation_count = 0;

void* counted_malloc(std::size_t size) noexcept
{
  ++allocation_count;
  return std::malloc(size == 0 ? 1 : size);
}

void counted_free(void* p) noexcept
{
  if (p) ++deallocation_count;
  std::free(p);
}

}  // namespace

// Every non-aligned form is replaced, so that no other allocation function (e.g. one provided by AddressSanitizer)
// allocates memory that these deallocation functions free.

void* operator new(std::size_t size)
{
  if (void* p = counted_malloc(size)) return p;
  throw std::bad_alloc();
}

void* operator new[](std::size_t size)
{
  if (void* p = counted_malloc(size)) return p;
  throw std::bad_alloc();
}

void* operator new(std::size_t size, std::nothrow_t const&) noexcept { return counted_malloc(size); }
void* operator new[](std::size_t size, std::nothrow_t const&) noexcept { return counted_malloc(size); }

void operator delete(void* p) noexcept { counted_free(p); }
void operator delete[](void* p) noexcept { counted_free(p); }
void operator delete(void* p, std::nothrow_t const&) noexcept { counted_free(p); }
void operator delete[](void* p, std::nothrow_t const&) noexcept { counted_free(p); }

#if __cpp_sized_deallocation >= 201309L
void operator delete(void* p, std::size_t) noexcept { counted_free(p); }
void operator delete[](void* p, std::size_t) noexcept { counted_free(p); }
#endif

namespace pf = yk::polyfill;

namespace {

int doubles(int x) { return 2 * x; }

struct Counter {
  int calls = 0;
  int operator()() { return ++calls; }
};

struct alignas(64) ThrowingExtendedAligned {
  explicit ThrowingExtendedAligned(int) { throw 42; }
  void operator()() const {}
};

}  // namespace

// [func.wrap.move.ctor] does not allow these targets to throw bad_alloc.
TEST_CASE("move_only_function: function pointers and reference_wrapper are stored without allocation")
{
  {
    // Checks that the counter works. GCC and Clang elide only allocations from new-expressions, not direct calls.
    std::size_t const before = allocation_count;
    ::operator delete(::operator new(1));
    CHECK(allocation_count == before + 1);
  }
  {
    std::size_t const before = allocation_count;
    pf::move_only_function<int(int)> f = doubles;
    pf::move_only_function<int(int)> g(std::move(f));
    f = std::move(g);
    f.swap(g);
    std::size_t const after = allocation_count;
    CHECK(after == before);
    CHECK(g(21) == 42);
  }
  {
    std::size_t const before = allocation_count;
    pf::move_only_function<int(int)> f(pf::in_place_type_t<int (*)(int)>{}, doubles);
    std::size_t const after = allocation_count;
    CHECK(after == before);
    CHECK(f(21) == 42);
  }
  {
    Counter c;
    std::size_t const before = allocation_count;
    pf::move_only_function<int()> f = std::ref(c);
    pf::move_only_function<int()> g(pf::in_place_type_t<std::reference_wrapper<Counter>>{}, c);
    f.swap(g);
    std::size_t const after = allocation_count;
    CHECK(after == before);
    CHECK(f() == 1);
    CHECK(g() == 2);
    CHECK(c.calls == 2);
  }
}

TEST_CASE("move_only_function: the allocation is released when constructing the target throws")
{
  // Before C++17 aligned new, a target with extended alignment is placed by hand in memory from operator new.
  std::size_t const allocations_before = allocation_count;
  std::size_t const deallocations_before = deallocation_count;
  bool threw = false;
  try {
    pf::move_only_function<void()> const f(pf::in_place_type_t<ThrowingExtendedAligned>{}, 0);
  } catch (int) {
    threw = true;
  }
  std::size_t const allocations = allocation_count - allocations_before;
  std::size_t const deallocations = deallocation_count - deallocations_before;
  CHECK(threw);
  CHECK(deallocations == allocations);
}
