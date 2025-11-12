#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

TEST_CASE("reference_constructs_from_temporary")
{
  // T is not reference type
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int const>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int, int const&&>::value == false);

  // T is non-const lvalue reference type
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int const>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&, int const&&>::value == false);

  // T is const lvalue reference type
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&, int const&&>::value == false);

  // T is non-const rvalue reference type
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int&&, int const&&>::value == false);

  // T is const rvalue reference type
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_constructs_from_temporary<int const&&, int const&&>::value == false);

  // TODO: more tests
}

TEST_CASE("reference_converts_from_temporary")
{
  // T is not reference type
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int const>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int, int const&&>::value == false);

  // T is non-const lvalue reference type
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int const>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&, int const&&>::value == false);

  // T is const lvalue reference type
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&, int const&&>::value == false);

  // T is non-const rvalue reference type
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int&&, int const&&>::value == false);

  // T is const rvalue reference type
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int>::value == true);        // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int const>::value == true);  // extended lifetime
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int const&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int&&>::value == false);
  STATIC_REQUIRE(pf::reference_converts_from_temporary<int const&&, int const&&>::value == false);

  // TODO: more tests
}
