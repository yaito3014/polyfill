#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/optional.hpp>

#include <compare>

namespace pf = yk::polyfill;

TEST_CASE("optional three-way comparison")
{
  // Three-way comparison between optional objects
  {
    pf::optional<int> a = 42;
    pf::optional<int> b = 42;
    pf::optional<int> c = 99;
    pf::optional<int> d;

    CHECK(((a <=> b) == std::strong_ordering::equal));
    CHECK(((a <=> c) == std::strong_ordering::less));
    CHECK(((c <=> a) == std::strong_ordering::greater));
    CHECK(((d <=> d) == std::strong_ordering::equal));
    CHECK(((d <=> a) == std::strong_ordering::less));
    CHECK(((a <=> d) == std::strong_ordering::greater));
  }

  // Three-way comparison with nullopt_t
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    CHECK(((a <=> pf::nullopt_holder::value) == std::strong_ordering::greater));
    CHECK(((b <=> pf::nullopt_holder::value) == std::strong_ordering::equal));
    CHECK(((pf::nullopt_holder::value <=> a) == std::strong_ordering::less));
    CHECK(((pf::nullopt_holder::value <=> b) == std::strong_ordering::equal));
  }

  // Three-way comparison with values
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    CHECK(((a <=> 42) == std::strong_ordering::equal));
    CHECK(((a <=> 99) == std::strong_ordering::less));
    CHECK(((a <=> 10) == std::strong_ordering::greater));
    CHECK(((b <=> 42) == std::strong_ordering::less));
  }
}
