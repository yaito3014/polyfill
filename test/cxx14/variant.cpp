#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("variant_size_v and variant_alternative_t")
{
  STATIC_REQUIRE(pf::variant_size_v<pf::variant<int>> == 1);
  STATIC_REQUIRE(pf::variant_size_v<pf::variant<int, double>> == 2);
  STATIC_REQUIRE(pf::variant_size_v<pf::variant<int, double, char>> == 3);
  STATIC_REQUIRE(pf::variant_size_v<pf::variant<int, double, char> const> == 3);

  STATIC_REQUIRE(std::is_same<pf::variant_alternative_t<0, pf::variant<int, double>>, int>::value);
  STATIC_REQUIRE(std::is_same<pf::variant_alternative_t<1, pf::variant<int, double>>, double>::value);
  STATIC_REQUIRE(std::is_same<pf::variant_alternative_t<0, pf::variant<int, double> const>, int>::value);
}

TEST_CASE("variant constexpr get")
{
  // get<I> const
  constexpr pf::variant<int, double> v1 = 42;
  STATIC_REQUIRE(pf::get<0>(v1) == 42);

  constexpr pf::variant<int, double> v2 = 3.14;
  STATIC_REQUIRE(pf::get<1>(v2) == 3.14);

  // get<T> const
  STATIC_REQUIRE(pf::get<int>(v1) == 42);
  STATIC_REQUIRE(pf::get<double>(v2) == 3.14);
}

TEST_CASE("variant constexpr visit")
{
  struct visitor {
    constexpr int operator()(int i) const { return i * 2; }
    constexpr int operator()(double d) const { return static_cast<int>(d); }
  };

  constexpr pf::variant<int, double> v = 21;
  STATIC_REQUIRE(pf::visit(visitor{}, v) == 42);

  constexpr pf::variant<int, double> v2 = 3.0;
  STATIC_REQUIRE(pf::visit(visitor{}, v2) == 3);
}

TEST_CASE("variant constexpr comparison")
{
  constexpr pf::variant<int, double> a = 42;
  constexpr pf::variant<int, double> b = 42;
  constexpr pf::variant<int, double> c = 99;
  constexpr pf::variant<int, double> d = 3.14;

  // same index, same value
  STATIC_REQUIRE((a == b));
  STATIC_REQUIRE((!(a != b)));

  // same index, different value
  STATIC_REQUIRE((a != c));
  STATIC_REQUIRE((a < c));
  STATIC_REQUIRE((a <= c));
  STATIC_REQUIRE((c > a));
  STATIC_REQUIRE((c >= a));

  // different index
  STATIC_REQUIRE((a != d));
  STATIC_REQUIRE((a < d));
  STATIC_REQUIRE((!(a > d)));
}
