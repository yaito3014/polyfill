#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/bits/swap.hpp>

namespace {

struct has_custom_swap {
  int value;
  bool swapped_via_adl;

  explicit has_custom_swap(int v) : value(v), swapped_via_adl(false) {}

  has_custom_swap(has_custom_swap const&) = default;
  has_custom_swap& operator=(has_custom_swap const&) = default;
};

void swap(has_custom_swap& a, has_custom_swap& b) noexcept
{
  int tmp = a.value;
  a.value = b.value;
  b.value = tmp;
  a.swapped_via_adl = true;
  b.swapped_via_adl = true;
}

struct no_custom_swap {
  int value;
};

}  // namespace

TEST_CASE("constexpr_swap")
{
  using yk::polyfill::detail::constexpr_swap;
  using yk::polyfill::detail::has_adl_swap;

  SECTION("has_adl_swap trait")
  {
    STATIC_REQUIRE(has_adl_swap<has_custom_swap>::value);
    STATIC_REQUIRE(!has_adl_swap<no_custom_swap>::value);
    STATIC_REQUIRE(!has_adl_swap<int>::value);
  }

  SECTION("calls ADL swap when available")
  {
    has_custom_swap a(1), b(2);
    constexpr_swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
    CHECK(a.swapped_via_adl);
    CHECK(b.swapped_via_adl);
  }

  SECTION("falls back to move-based swap without ADL swap")
  {
    no_custom_swap a{1}, b{2};
    constexpr_swap(a, b);
    CHECK(a.value == 2);
    CHECK(b.value == 1);
  }

  SECTION("works with scalars")
  {
    int a = 10, b = 20;
    constexpr_swap(a, b);
    CHECK(a == 20);
    CHECK(b == 10);
  }

  SECTION("works with pointers")
  {
    int x = 1, y = 2;
    int* a = &x;
    int* b = &y;
    constexpr_swap(a, b);
    CHECK(a == &y);
    CHECK(b == &x);
  }
}
