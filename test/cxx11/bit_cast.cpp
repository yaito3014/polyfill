#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/bit.hpp>

#include <cstdint>
#include <limits>

namespace pf = yk::polyfill;

TEST_CASE("bit_cast")
{
  SECTION("float to uint32_t")
  {
    if YK_POLYFILL_CXX17_CONSTEXPR (std::numeric_limits<float>::is_iec559) {
      float f = 1.0f;
      std::uint32_t u = pf::bit_cast<std::uint32_t>(f);
      CHECK(u == 0x3F800000u);
    }
  }

  SECTION("uint32_t to float")
  {
    if YK_POLYFILL_CXX17_CONSTEXPR (std::numeric_limits<float>::is_iec559) {
      std::uint32_t u = 0x3F800000u;
      float f = pf::bit_cast<float>(u);
      CHECK(f == 1.0f);
    }
  }

  SECTION("round trip")
  {
    double d = 3.14;
    std::uint64_t u = pf::bit_cast<std::uint64_t>(d);
    double d2 = pf::bit_cast<double>(u);
    CHECK(d == d2);
  }
}
