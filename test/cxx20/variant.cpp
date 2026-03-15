#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

#include <compare>
#include <utility>

namespace pf = yk::polyfill;

TEST_CASE("variant constexpr emplace")
{
  // emplace by index
  {
    constexpr auto result = [] {
      pf::variant<int, double> v = 0;
      v.emplace<1>(3.14);
      return v;
    }();
    STATIC_REQUIRE(result.index() == 1);
    STATIC_REQUIRE(pf::get<1>(result) == 3.14);
  }
  // emplace by type
  {
    constexpr auto result = [] {
      pf::variant<int, double> v = 0;
      v.emplace<double>(2.5);
      return v;
    }();
    STATIC_REQUIRE(result.index() == 1);
    STATIC_REQUIRE(pf::get<1>(result) == 2.5);
  }
}

TEST_CASE("variant constexpr assignment")
{
  // converting assignment
  {
    constexpr auto result = [] {
      pf::variant<int, double> v = 0;
      v = 3.14;
      return v;
    }();
    STATIC_REQUIRE(result.index() == 1);
    STATIC_REQUIRE(pf::get<1>(result) == 3.14);
  }
  // copy assignment
  {
    constexpr auto result = [] {
      pf::variant<int, double> v1 = 42;
      pf::variant<int, double> v2 = 3.14;
      v1 = v2;
      return v1;
    }();
    STATIC_REQUIRE(result.index() == 1);
    STATIC_REQUIRE(pf::get<1>(result) == 3.14);
  }
  // move assignment
  {
    constexpr auto result = [] {
      pf::variant<int, double> v1 = 42;
      pf::variant<int, double> v2 = 3.14;
      v1 = std::move(v2);
      return v1;
    }();
    STATIC_REQUIRE(result.index() == 1);
    STATIC_REQUIRE(pf::get<1>(result) == 3.14);
  }
}

TEST_CASE("variant constexpr swap")
{
  constexpr auto result = [] {
    pf::variant<int, double> v1 = 42;
    pf::variant<int, double> v2 = 3.14;
    v1.swap(v2);
    return std::pair{v1.index(), v2.index()};
  }();
  STATIC_REQUIRE(result.first == 1);
  STATIC_REQUIRE(result.second == 0);
}

TEST_CASE("variant constexpr three-way comparison")
{
  constexpr pf::variant<int, double> a = 42;
  constexpr pf::variant<int, double> b = 42;
  constexpr pf::variant<int, double> c = 99;

  STATIC_REQUIRE((a <=> b) == std::strong_ordering::equal);
  STATIC_REQUIRE((a <=> c) == std::strong_ordering::less);
  STATIC_REQUIRE((c <=> a) == std::strong_ordering::greater);

  STATIC_REQUIRE((pf::monostate{} <=> pf::monostate{}) == std::strong_ordering::equal);
}
