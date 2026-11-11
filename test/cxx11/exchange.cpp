#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/utility.hpp>

#include <memory>

namespace pf = yk::polyfill;

TEST_CASE("exchange")
{
  int x = 33;
  int y = pf::exchange(x, 4);
  CHECK(x == 4);
  CHECK(y == 33);
  // [utility.exchange]: is_nothrow_move_constructible_v<T> && is_nothrow_assignable_v<T&, U>
  STATIC_REQUIRE(noexcept(pf::exchange(x, 4)));

  std::unique_ptr<int> ptr(new int(1));
  std::unique_ptr<int> old = pf::exchange(ptr, std::unique_ptr<int>(new int(2)));
  CHECK(*old == 1);
  CHECK(*ptr == 2);
}
