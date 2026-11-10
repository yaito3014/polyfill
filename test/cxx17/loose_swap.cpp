#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/concepts.hpp>

#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

namespace {

template<class T, class = void>
struct can_loose_swap : std::false_type {};

template<class T>
struct can_loose_swap<T, pf::void_t<decltype(pf::detail::loose_swap(std::declval<T&>(), std::declval<T&>()))>> : std::true_type {};

// Swappable per [swappable.requirements] although ranges::swap rejects them. This needs C++17,
// where std::swap is constrained on move-constructibility.
struct id_base {
  int id;
};

void swap(id_base& a, id_base& b) noexcept
{
  int const tmp = a.id;
  a.id = b.id;
  b.id = tmp;
}

struct base_swap_only : id_base {
  explicit base_swap_only(int i) : id_base{i} {}
  base_swap_only(base_swap_only&&) = delete;
};

template<class T>
struct is_template_swappable : std::false_type {};

template<class T, typename std::enable_if<is_template_swappable<T>::value, std::nullptr_t>::type = nullptr>
void swap(T& a, T& b) noexcept
{
  int const tmp = a.id;
  a.id = b.id;
  b.id = tmp;
}

struct template_swap_only {
  int id;
  explicit template_swap_only(int i) : id(i) {}
  template_swap_only(template_swap_only&&) = delete;
};

template<>
struct is_template_swappable<template_swap_only> : std::true_type {};

struct no_move {
  no_move() = default;
  no_move(no_move&&) = delete;
};

}  // namespace

TEST_CASE("loose_swap - std::swap context fallback")
{
  using pf::detail::loose_swap;

  SECTION("swap for a base class")
  {
    base_swap_only a(1), b(2);
    loose_swap(a, b);
    CHECK(a.id == 2);
    CHECK(b.id == 1);
    STATIC_REQUIRE(noexcept(loose_swap(a, b)));
  }

  SECTION("constrained swap template")
  {
    template_swap_only a(1), b(2);
    loose_swap(a, b);
    CHECK(a.id == 2);
    CHECK(b.id == 1);
  }

  SECTION("neither applies")
  {
    STATIC_REQUIRE_FALSE(can_loose_swap<no_move>::value);
    STATIC_REQUIRE(can_loose_swap<base_swap_only>::value);
  }
}
