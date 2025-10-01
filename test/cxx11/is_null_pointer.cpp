#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/cxx11/is_null_pointer.hpp>

namespace pf = yk::polyfill;

TEST_CASE("is_null_pointer")
{
  STATIC_CHECK(pf::is_null_pointer<std::nullptr_t>::value == true);
  STATIC_CHECK(pf::is_null_pointer<std::nullptr_t const>::value == true);
  STATIC_CHECK(pf::is_null_pointer<std::nullptr_t volatile>::value == true);
  STATIC_CHECK(pf::is_null_pointer<std::nullptr_t const volatile>::value == true);

  STATIC_CHECK(pf::is_null_pointer<int>::value == false);
  STATIC_CHECK(pf::is_null_pointer<int*>::value == false);
}
