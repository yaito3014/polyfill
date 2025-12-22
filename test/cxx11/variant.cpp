#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

struct NotTriviallyDestructible {
  ~NotTriviallyDestructible() {}
};

TEST_CASE("variant destruction")
{
  STATIC_REQUIRE(std::is_trivially_destructible<pf::variant<int>>::value);
  STATIC_REQUIRE(!std::is_trivially_destructible<pf::variant<NotTriviallyDestructible>>::value);

  STATIC_REQUIRE(std::is_destructible<pf::variant<int>>::value);
  STATIC_REQUIRE(std::is_destructible<pf::variant<NotTriviallyDestructible>>::value);
}

struct NotDefaultConstructible {
  NotDefaultConstructible(int) {}
};

TEST_CASE("variant in-place index construction")
{
  STATIC_REQUIRE(std::is_constructible<pf::variant<int>, pf::in_place_index_t<0>, int>::value);

  STATIC_REQUIRE(!std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>, int>::value);

  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_index_t<0>, int>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_index_t<1>, double>::value);

  {
    pf::variant<int> vi(pf::in_place_index<0>, 42);
    CHECK(vi.index() == 0);
  }

  {
    pf::variant<int, double> vid(pf::in_place_index<0>, 42);
    CHECK(vid.index() == 0);
  }

  {
    pf::variant<int, double> vid(pf::in_place_index<1>, 3.14);
    CHECK(vid.index() == 1);
  }
}

TEST_CASE("dummy")
{
  pf::variant<int> vi;
  pf::variant<NotTriviallyDestructible> vntd;
}
