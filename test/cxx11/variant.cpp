#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/variant.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

struct ThrowsOnConstruction {
  ThrowsOnConstruction() { throw std::exception{}; }
};

void make_valueless(pf::variant<int, ThrowsOnConstruction>& var)
{
  try {
    var.emplace<1>();
  } catch (...) {
  }
}

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
  STATIC_REQUIRE(std::is_default_constructible<pf::variant<int, NotDefaultConstructible>>::value);
  STATIC_REQUIRE(!std::is_default_constructible<pf::variant<NotDefaultConstructible, int>>::value);

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
  {
    pf::variant<int, ThrowsOnConstruction> vit = 42;
    make_valueless(vit);
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
  {
    pf::variant<int, double> vid = 42;
    CHECK(pf::get<0>(vid) == 42);
    CHECK_THROWS(pf::get<1>(vid));
  }
  {
    pf::variant<int, double> vid = 3.14;
    CHECK(pf::get<1>(vid) == 3.14);
    CHECK_THROWS(pf::get<0>(vid));
  }
}

TEST_CASE("variant copy construction")
{
  SECTION("trivial")
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_trivially_copy_constructible<V>::value);
    V a = 42;
    V b = a;
    CHECK(pf::get<0>(b) == 42);
  }
  SECTION("non-trivial")
  {
    struct NonTriviallyCopyConstructible {
      NonTriviallyCopyConstructible(NonTriviallyCopyConstructible const&) {}
    };
    using V = pf::variant<int, NonTriviallyCopyConstructible>;
    STATIC_REQUIRE(!std::is_trivially_copy_constructible<V>::value);
    STATIC_REQUIRE(std::is_copy_constructible<V>::value);
    V a = 42;
    V b = a;
    CHECK(pf::get<0>(b) == 42);
  }
  {
    pf::variant<int, ThrowsOnConstruction> a = 42;
    make_valueless(a);
    pf::variant<int, ThrowsOnConstruction> b = a;
    CHECK(b.valueless_by_exception());
  }
}

TEST_CASE("variant move construction")
{
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_trivially_move_constructible<V>::value);
    V a = 42;
    V b = std::move(a);
    CHECK(pf::get<0>(b) == 42);
  }
  {
    struct NonTriviallyMoveConstructible {
      NonTriviallyMoveConstructible(NonTriviallyMoveConstructible&&) {}
    };
    using V = pf::variant<int, NonTriviallyMoveConstructible>;
    STATIC_REQUIRE(!std::is_trivially_move_constructible<V>::value);
    STATIC_REQUIRE(std::is_move_constructible<V>::value);
    V a = 42;
    V b = std::move(a);
    CHECK(pf::get<0>(b) == 42);
  }
  {
    pf::variant<int, ThrowsOnConstruction> a = 42;
    make_valueless(a);
    pf::variant<int, ThrowsOnConstruction> b = std::move(a);
    CHECK(b.valueless_by_exception());
  }
}

TEST_CASE("variant copy assignment")
{
  SECTION("trivial case")
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_trivially_copy_assignable<V>::value);
    SECTION("same alternative")
    {
      V a = 12;
      V b = 34;
      a = b;
      CHECK(pf::get<0>(a) == 34);
      CHECK(pf::get<0>(b) == 34);
    }
    SECTION("different alternative")
    {
      V a = 12;
      V b = 3.14;
      a = b;
      CHECK(pf::get<1>(a) == 3.14);
      CHECK(pf::get<1>(b) == 3.14);
    }
  }
  SECTION("non-trivial case")
  {
    struct NonTriviallyCopyAssignable {
      NonTriviallyCopyAssignable& operator=(NonTriviallyCopyAssignable const&) { return *this; }
    };
    using V = pf::variant<int, NonTriviallyCopyAssignable>;
    STATIC_REQUIRE(!std::is_trivially_copy_assignable<V>::value);
    STATIC_REQUIRE(std::is_copy_assignable<V>::value);
    SECTION("same alternative")
    {
      V a = 12;
      V b = 34;
      a = b;
      CHECK(pf::get<0>(a) == 34);
      CHECK(pf::get<0>(b) == 34);
    }
    SECTION("different alternative")
    {
      V a = NonTriviallyCopyAssignable{};
      V b = 42;
      a = b;
      CHECK(pf::get<0>(a) == 42);
      CHECK(pf::get<0>(b) == 42);
    }
  }
}
