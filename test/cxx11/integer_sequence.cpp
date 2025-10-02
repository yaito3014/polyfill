#include <catch2/catch.hpp>

#include <yk/polyfill/cxx11/integer_sequence.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

TEST_CASE("integer_sequence")
{
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 0>, pf::integer_sequence<int>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 1>, pf::integer_sequence<int, 0>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 2>, pf::integer_sequence<int, 0, 1>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 3>, pf::integer_sequence<int, 0, 1, 2>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 4>, pf::integer_sequence<int, 0, 1, 2, 3>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 5>, pf::integer_sequence<int, 0, 1, 2, 3, 4>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 6>, pf::integer_sequence<int, 0, 1, 2, 3, 4, 5>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_integer_sequence<int, 7>, pf::integer_sequence<int, 0, 1, 2, 3, 4, 5, 6>>::value);

  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<0>, pf::index_sequence<>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<1>, pf::index_sequence<0>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<2>, pf::index_sequence<0, 1>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<3>, pf::index_sequence<0, 1, 2>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<4>, pf::index_sequence<0, 1, 2, 3>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<5>, pf::index_sequence<0, 1, 2, 3, 4>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<6>, pf::index_sequence<0, 1, 2, 3, 4, 5>>::value);
  STATIC_REQUIRE(std::is_same<pf::make_index_sequence<7>, pf::index_sequence<0, 1, 2, 3, 4, 5, 6>>::value);

  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<>, pf::index_sequence<>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int>, pf::index_sequence<0>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int>, pf::index_sequence<0, 1>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int, int>, pf::index_sequence<0, 1, 2>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int, int, int>, pf::index_sequence<0, 1, 2, 3>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int, int, int, int>, pf::index_sequence<0, 1, 2, 3, 4>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int, int, int, int, int>, pf::index_sequence<0, 1, 2, 3, 4, 5>>::value);
  STATIC_REQUIRE(std::is_same<pf::index_sequence_for<int, int, int, int, int, int, int>, pf::index_sequence<0, 1, 2, 3, 4, 5, 6>>::value);
}
