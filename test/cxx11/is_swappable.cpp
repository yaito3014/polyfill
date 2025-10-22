#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

struct has_adl_swap {
  friend void swap(has_adl_swap&, has_adl_swap&) {}
};

struct has_deleted_adl_swap {
  friend void swap(has_deleted_adl_swap&, has_deleted_adl_swap&) = delete;
};

struct has_potentially_throwing_swap {
  friend void swap(has_potentially_throwing_swap&, has_potentially_throwing_swap&) noexcept(false) {}
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

  STATIC_REQUIRE(pf::is_nothrow_swappable<int>::value == true);
  STATIC_REQUIRE(pf::is_nothrow_swappable<has_potentially_throwing_swap>::value == false);
}
