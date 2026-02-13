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

struct NotTriviallyDestructible {
  ~NotTriviallyDestructible() {}
};

TEST_CASE("variant default construction and destruction")
{
  // default construction
  STATIC_REQUIRE(std::is_default_constructible<pf::variant<int>>::value);
  STATIC_REQUIRE(!std::is_default_constructible<pf::variant<NotDefaultConstructible>>::value);

  // trivial destruction
  STATIC_REQUIRE(std::is_trivially_destructible<pf::variant<int>>::value);
  STATIC_REQUIRE(!std::is_trivially_destructible<pf::variant<NotTriviallyDestructible>>::value);

  // destruction
  STATIC_REQUIRE(std::is_destructible<pf::variant<int>>::value);
  STATIC_REQUIRE(std::is_destructible<pf::variant<NotTriviallyDestructible>>::value);

  {
    pf::variant<int> vi;
  }
  {
    pf::variant<NotTriviallyDestructible> vntd;
  }
}

TEST_CASE("variant in-place index construction")
{
  STATIC_REQUIRE(std::is_constructible<pf::variant<int>, pf::in_place_index_t<0>, int>::value);

  STATIC_REQUIRE(!std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_index_t<0>, int>::value);

  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_index_t<0>, int>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_index_t<1>, double>::value);

  {
    pf::variant<int> vi(pf::in_place_index_t<0>{}, 42);
    CHECK(vi.index() == 0);
  }

  {
    pf::variant<int, double> vid(pf::in_place_index_t<0>{}, 42);
    CHECK(vid.index() == 0);
  }

  {
    pf::variant<int, double> vid(pf::in_place_index_t<1>{}, 3.14);
    CHECK(vid.index() == 1);
  }
}

TEST_CASE("variant generic construction")
{
  {
    STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, int>::value);
    pf::variant<int, double> vid = 42;
    CHECK(vid.index() == 0);
  }
  {
    STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, double>::value);
    pf::variant<int, double> vid = 3.14;
    CHECK(vid.index() == 1);
  }

  // ambiguous
  STATIC_REQUIRE(!std::is_constructible<pf::variant<long, long long>, int>::value);

  // narrowing
  STATIC_REQUIRE(!std::is_constructible<pf::variant<int>, double>::value);
}

TEST_CASE("variant get")
{
  pf::variant<int, double> vid;
  vid = 42;
  CHECK(pf::get<0>(vid) == 42);
  vid = 3.14;
  CHECK(pf::get<1>(vid) == 3.14);
}

struct NonTriviallyCopyConstructible {
  NonTriviallyCopyConstructible() = default;
  NonTriviallyCopyConstructible(NonTriviallyCopyConstructible const&) {}
  NonTriviallyCopyConstructible(NonTriviallyCopyConstructible&&) = default;
};

TEST_CASE("variant copy construction")
{
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_trivially_copy_constructible<V>::value);
    V a = 42;
    V b = a;
    CHECK(pf::get<0>(b) == 42);
  }
  {
    using V = pf::variant<int, NonTriviallyCopyConstructible>;
    STATIC_REQUIRE(!std::is_trivially_copy_constructible<NonTriviallyCopyConstructible>::value);
    STATIC_REQUIRE(std::is_copy_constructible<NonTriviallyCopyConstructible>::value);
    V a = 42;
    V b = a;
    CHECK(pf::get<0>(b) == 42);
  }
}
