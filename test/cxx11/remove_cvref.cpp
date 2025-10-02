#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/remove_cvref.hpp>

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
