#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/concepts.hpp>
#include <yk/polyfill/type_traits.hpp>

#include <string>
#include <utility>

namespace pf = yk::polyfill;

namespace {

struct has_adl_swap {
  int value;
  bool via_adl;
};

void swap(has_adl_swap& a, has_adl_swap& b)
{
  std::swap(a.value, b.value);
  a.via_adl = true;
  b.via_adl = true;
}

// The std::swap context finds these through ADL on the pointee and element type, and prefers them
// to std::swap as non-templates.
struct pointee {
  bool via_pointer_swap;
};

void swap(pointee*& a, pointee*& b)
{
  pointee* const tmp = a;
  a = b;
  b = tmp;
  a->via_pointer_swap = true;
  b->via_pointer_swap = true;
}

struct element {
  int value;
  bool via_array_swap;
};

void swap(element (&a)[1], element (&b)[1])
{
  std::swap(a[0].value, b[0].value);
  a[0].via_array_swap = true;
  b[0].via_array_swap = true;
}

struct std_swap_specialized {
  int value;
  bool via_specialization;
};

}  // namespace

#if __cplusplus < 202002L
// Allowed before C++20 ([namespace.std]); since C++20 adding it to namespace std is undefined.
namespace std {

template<>
void swap<std_swap_specialized>(std_swap_specialized& a, std_swap_specialized& b) noexcept
{
  int const tmp = a.value;
  a.value = b.value;
  b.value = tmp;
  a.via_specialization = true;
  b.via_specialization = true;
}

}  // namespace std
#endif

namespace generic {

struct widget {
  int value;
  bool via_template;
};

// ambiguous with std::swap, so the std::swap context rejects widget; ranges::swap exchanges it
template<class T>
void swap(T& a, T& b)
{
  a.via_template = true;
  b.via_template = true;
}

}  // namespace generic

TEST_CASE("loose_swap")
{
  using pf::detail::loose_swap;

  SECTION("exchanges scalars and arrays")
  {
    int a = 1, b = 2;
    loose_swap(a, b);
    CHECK(a == 2);
    CHECK(b == 1);
    STATIC_REQUIRE(noexcept(loose_swap(a, b)));

    int x[2] = {1, 2}, y[2] = {3, 4};
    loose_swap(x, y);
    CHECK(x[0] == 3);
    CHECK(y[1] == 2);
  }

  SECTION("calls an ADL swap")
  {
    has_adl_swap a{1, false}, b{2, false};
    loose_swap(a, b);
    CHECK(a.value == 2);
    CHECK(a.via_adl);
    STATIC_REQUIRE_FALSE(noexcept(loose_swap(a, b)));
  }

  SECTION("swaps std types")
  {
    std::string a = "a", b = "b";
    loose_swap(a, b);
    CHECK(a == "b");
    CHECK(b == "a");
  }

  SECTION("calls an ADL swap for pointers and arrays")
  {
    pointee x{false}, y{false};
    pointee* p = &x;
    pointee* q = &y;
    loose_swap(p, q);
    CHECK(p == &y);
    CHECK(q == &x);
    CHECK(x.via_pointer_swap);

    element a[1] = {{1, false}}, b[1] = {{2, false}};
    loose_swap(a, b);
    CHECK(a[0].value == 2);
    CHECK(b[0].value == 1);
    CHECK(a[0].via_array_swap);
  }

#if __cplusplus < 202002L
  SECTION("calls an explicit specialization of std::swap")
  {
    std_swap_specialized a{1, false}, b{2, false};
    loose_swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    CHECK(a.via_specialization);
  }
#endif

  SECTION("accepts what only ranges::swap accepts")
  {
    STATIC_REQUIRE_FALSE(pf::is_swappable<generic::widget>::value);
    generic::widget a{1, false}, b{2, false};
    loose_swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    CHECK_FALSE(a.via_template);
  }
}
