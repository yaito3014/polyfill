#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/utility.hpp>

namespace pf = yk::polyfill;

TEST_CASE("exchange")
{
  int x = 33;
  int y = pf::exchange(x, 4);
  CHECK(x == 4);
  CHECK(y == 33);
}
