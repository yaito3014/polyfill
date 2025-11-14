#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/is_convertible_without_narrowing.hpp>

#include <cstdint>

namespace pf = yk::polyfill;

TEST_CASE("is_convertible_without_narrowing")
{
  // conversion between integer types
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<std::int16_t, std::int32_t>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<std::int32_t, std::int16_t>::value);

  // conversion between floating-point types
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<float, double>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<double, float>::value);

  // conversion between integer and floating-point types
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<int, double>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<double, int>::value);

  // conversion including void
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<int, void>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<void, int>::value);
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<void, void>::value);

  // conversion including function type
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<void(void), void (*)(void)>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<void (*)(void), void(void)>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<void(void), void(void)>::value);

  // conversion including reference type
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<int, int&>::value);
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<int&, int>::value);
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<int&, int&>::value);

  // conversion including unbouded array type
  STATIC_REQUIRE(pf::extension::is_convertible_without_narrowing<int[], int*>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<int*, int[]>::value);
  STATIC_REQUIRE(!pf::extension::is_convertible_without_narrowing<int[], int[]>::value);
}
