#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/cxx11/is_null_pointer.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_null_pointer")
{
  STATIC_REQUIRE(pf::is_null_pointer<std::nullptr_t>::value == true);
  STATIC_REQUIRE(pf::is_null_pointer<std::nullptr_t const>::value == true);
  STATIC_REQUIRE(pf::is_null_pointer<std::nullptr_t volatile>::value == true);
  STATIC_REQUIRE(pf::is_null_pointer<std::nullptr_t const volatile>::value == true);

  STATIC_REQUIRE(pf::is_null_pointer<int>::value == false);
  STATIC_REQUIRE(pf::is_null_pointer<int*>::value == false);
}
