#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

struct NotDefaultConstructible {
  NotDefaultConstructible(int) {}
};

TEST_CASE("variant in-place index construction")
{
  STATIC_REQUIRE(std::is_constructible<pf::variant<int>, pf::in_place_index_t<0>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<int>, pf::in_place_index_t<0>, int>::value);

  STATIC_REQUIRE(!std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>, int>::value);
}