#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/memory.hpp>

#include <type_traits>

#include <cstddef>
#include <cstring>

namespace pf = yk::polyfill;

TEST_CASE("make_unique")
{
  {
    auto x = pf::make_unique<int>(42);
    STATIC_REQUIRE(std::is_same<decltype(x), pf::unique_ptr<int>>::value);
    CHECK(*x == 42);
  }
  {
    auto x = pf::make_unique<int[]>(5);
    STATIC_REQUIRE(std::is_same<decltype(x), pf::unique_ptr<int[]>>::value);
  }
}

namespace {

// Hands out storage filled with a non-zero pattern, so that elements left default-initialized do not read as zero.
struct PoisonedStorage {
  int value;

  static void* operator new[](std::size_t size)
  {
    void* const p = ::operator new[](size);
    std::memset(p, 0xA5, size);
    return p;
  }

  static void operator delete[](void* p) noexcept { ::operator delete[](p); }
};

}  // namespace

TEST_CASE("make_unique value-initializes array elements")
{
  auto p = pf::make_unique<PoisonedStorage[]>(4);
  for (std::size_t i = 0; i < 4; ++i) {
    CHECK(p[i].value == 0);
  }
}
