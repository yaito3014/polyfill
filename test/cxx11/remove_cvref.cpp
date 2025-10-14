#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("remove_cvref")
{
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int volatile>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const volatile>::type, int>::value);

  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int volatile&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const volatile&>::type, int>::value);

  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int&&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const&&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int volatile&&>::type, int>::value);
  STATIC_REQUIRE(std::is_same<pf::remove_cvref<int const volatile&&>::type, int>::value);
}
