#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/memory.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

namespace {

// Non-movable deleters (held by reference) that are swappable per [swappable.requirements] although
// ranges::swap rejects them: one's swap is an overload for a base class that adds no state, the
// other's is a constrained template that ties with ranges::swap's deleted swap(T&, T&). This needs
// C++17, where std::swap is constrained on move-constructibility; in C++14 the base-class swap
// loses to std::swap and the template one is ambiguous with it.
struct id_base {
  int id;
};

void swap(id_base& a, id_base& b) noexcept
{
  int const tmp = a.id;
  a.id = b.id;
  b.id = tmp;
}

struct base_swap_deleter : id_base {
  explicit base_swap_deleter(int i) : id_base{i} {}
  base_swap_deleter(base_swap_deleter&&) = delete;
  void operator()(int* p) const noexcept { delete p; }
};

struct base_swap_array_deleter : id_base {
  explicit base_swap_array_deleter(int i) : id_base{i} {}
  base_swap_array_deleter(base_swap_array_deleter&&) = delete;
  void operator()(int* p) const noexcept { delete[] p; }
};

template<class T>
struct is_template_swap_deleter : std::false_type {};

template<class T, typename std::enable_if<is_template_swap_deleter<T>::value, std::nullptr_t>::type = nullptr>
void swap(T& a, T& b) noexcept
{
  int const tmp = a.id;
  a.id = b.id;
  b.id = tmp;
}

struct template_swap_deleter {
  int id;
  explicit template_swap_deleter(int i) : id(i) {}
  template_swap_deleter(template_swap_deleter&&) = delete;
  void operator()(int* p) const noexcept { delete p; }
};

template<>
struct is_template_swap_deleter<template_swap_deleter> : std::true_type {};

}  // namespace

TEST_CASE("unique_ptr - swap of non-movable reference deleters")
{
  SECTION("swap for a base class")
  {
    base_swap_deleter d1(1), d2(2);
    pf::unique_ptr<int, base_swap_deleter&> a(new int(10), d1), b(new int(20), d2);
    a.swap(b);
    CHECK(*a == 20);
    CHECK(*b == 10);
    CHECK(d1.id == 2);
    CHECK(d2.id == 1);
  }

  SECTION("constrained swap template")
  {
    template_swap_deleter d1(1), d2(2);
    pf::unique_ptr<int, template_swap_deleter&> a(new int(10), d1), b(new int(20), d2);
    a.swap(b);
    CHECK(*a == 20);
    CHECK(*b == 10);
    CHECK(d1.id == 2);
    CHECK(d2.id == 1);
  }

  SECTION("array specialization")
  {
    base_swap_array_deleter d1(1), d2(2);
    pf::unique_ptr<int[], base_swap_array_deleter&> a(new int[1]{10}, d1), b(new int[1]{20}, d2);
    a.swap(b);
    CHECK(a[0] == 20);
    CHECK(b[0] == 10);
    CHECK(d1.id == 2);
    CHECK(d2.id == 1);
  }
}
