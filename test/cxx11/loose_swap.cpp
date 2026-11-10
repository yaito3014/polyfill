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

}  // namespace

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
