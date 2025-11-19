#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/toptional.hpp>

namespace ext = yk::polyfill::extension;

TEST_CASE("toptional")
{
  STATIC_REQUIRE(sizeof(ext::toptional<int>) == sizeof(int));
  {
    ext::toptional<int> opt;
    CHECK(!opt.has_value());
  }
  {
    ext::toptional<int> opt = 42;
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }
  CHECK_THROWS_AS(ext::toptional<int>{0}, ext::bad_toptional_initialization);

  STATIC_REQUIRE(sizeof(ext::toptional<int*>) == sizeof(int*));
  {
    int* ptr = new int(42);
    ext::toptional<int*> opt = ptr;
    CHECK(opt.has_value());
    CHECK(*opt == ptr);
    delete ptr;
  }
  CHECK_THROWS_AS(ext::toptional<int*>{nullptr}, ext::bad_toptional_initialization);
}
