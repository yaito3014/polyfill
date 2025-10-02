#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/is_swappable.hpp>

namespace pf = yk::polyfill;

struct has_adl_swap {
  friend void swap(has_adl_swap&, has_adl_swap&) {}
};

struct has_deleted_adl_swap {
  friend void swap(has_deleted_adl_swap&, has_deleted_adl_swap&) = delete;
};

namespace pf = yk::polyfill;

TEST_CASE("is_swappable")
{
  STATIC_REQUIRE(pf::is_swappable<int>::value == true);
  STATIC_REQUIRE(pf::is_swappable<int&>::value == true);
  STATIC_REQUIRE(pf::is_swappable<int&&>::value == true);

  STATIC_REQUIRE(pf::is_swappable<has_adl_swap>::value == true);

  STATIC_REQUIRE(pf::is_swappable<has_deleted_adl_swap>::value == false);

  STATIC_REQUIRE(pf::is_swappable<void>::value == false);
}
