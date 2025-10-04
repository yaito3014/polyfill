#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/cxx11/make_unique.hpp>
#include <yk/polyfill/cxx11/void_t.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("make_unique")
{
  {
    auto x = pf::make_unique<int>(42);
    STATIC_REQUIRE(std::is_same<decltype(x), std::unique_ptr<int>>::value);
    CHECK(*x == 42);
  }
  {
    auto x = pf::make_unique<int[]>(5);
    STATIC_REQUIRE(std::is_same<decltype(x), std::unique_ptr<int[]>>::value);
  }
}
