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

struct NonTriviallyDestructible {
  ~NonTriviallyDestructible() {}
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
  STATIC_REQUIRE(!std::is_trivially_destructible<pf::variant<NonTriviallyDestructible>>::value);

  // destruction
  STATIC_REQUIRE(std::is_destructible<pf::variant<int>>::value);
  STATIC_REQUIRE(std::is_destructible<pf::variant<NonTriviallyDestructible>>::value);

  {
    pf::variant<int> vi;
  }
  {
    pf::variant<NonTriviallyDestructible> vntd;
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

TEST_CASE("variant emplace")
{
  SECTION("same alternative")
  {
    pf::variant<int, double> v = 42;
    CHECK(v.index() == 0);
    auto& ref = v.emplace<0>(99);
    CHECK(v.index() == 0);
    CHECK(pf::get<0>(v) == 99);
    CHECK(ref == 99);
  }
  SECTION("different alternative")
  {
    pf::variant<int, double> v = 42;
    CHECK(v.index() == 0);
    auto& ref = v.emplace<1>(3.14);
    CHECK(v.index() == 1);
    CHECK(pf::get<1>(v) == 3.14);
    CHECK(ref == 3.14);
  }
  SECTION("non-trivially destructible")
  {
    pf::variant<int, NonTriviallyDestructible> v = 42;
    v.emplace<1>();
    CHECK(v.index() == 1);
    v.emplace<0>(10);
    CHECK(v.index() == 0);
    CHECK(pf::get<0>(v) == 10);
  }
  SECTION("from valueless")
  {
    pf::variant<int, ThrowsOnConstruction> v = 42;
    make_valueless(v);
    CHECK(v.valueless_by_exception());
    v.emplace<0>(7);
    CHECK(!v.valueless_by_exception());
    CHECK(v.index() == 0);
    CHECK(pf::get<0>(v) == 7);
  }
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
      NonTriviallyCopyConstructible() = default;
      NonTriviallyCopyConstructible(NonTriviallyCopyConstructible const&) {}
      NonTriviallyCopyConstructible(NonTriviallyCopyConstructible&&) = default;
      NonTriviallyCopyConstructible& operator=(NonTriviallyCopyConstructible const&) = default;
      NonTriviallyCopyConstructible& operator=(NonTriviallyCopyConstructible&&) = default;
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
      NonTriviallyMoveConstructible() = default;

      NonTriviallyMoveConstructible(NonTriviallyMoveConstructible const&) = default;
      NonTriviallyMoveConstructible(NonTriviallyMoveConstructible&&) {}
      NonTriviallyMoveConstructible& operator=(NonTriviallyMoveConstructible const&) = default;
      NonTriviallyMoveConstructible& operator=(NonTriviallyMoveConstructible&&) = default;
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
      NonTriviallyCopyAssignable() = default;
      NonTriviallyCopyAssignable(NonTriviallyCopyAssignable const&) = default;
      NonTriviallyCopyAssignable(NonTriviallyCopyAssignable&&) = default;
      NonTriviallyCopyAssignable& operator=(NonTriviallyCopyAssignable const&) { return *this; }
      NonTriviallyCopyAssignable& operator=(NonTriviallyCopyAssignable&&) = default;
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

TEST_CASE("variant move assignment")
{
  SECTION("trivial case")
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_trivially_move_assignable<V>::value);
    SECTION("same alternative")
    {
      V a = 12;
      V b = 34;
      a = std::move(b);
      CHECK(pf::get<0>(a) == 34);
    }
    SECTION("different alternative")
    {
      V a = 12;
      V b = 3.14;
      a = std::move(b);
      CHECK(pf::get<1>(a) == 3.14);
    }
  }
  SECTION("non-trivial case")
  {
    struct NonTriviallyMoveAssignable {
      NonTriviallyMoveAssignable() = default;
      NonTriviallyMoveAssignable(NonTriviallyMoveAssignable const&) = default;
      NonTriviallyMoveAssignable(NonTriviallyMoveAssignable&&) = default;
      NonTriviallyMoveAssignable& operator=(NonTriviallyMoveAssignable const&) = default;
      NonTriviallyMoveAssignable& operator=(NonTriviallyMoveAssignable&&) { return *this; }
    };
    using V = pf::variant<int, NonTriviallyMoveAssignable>;
    STATIC_REQUIRE(!std::is_trivially_move_assignable<V>::value);
    STATIC_REQUIRE(std::is_move_assignable<V>::value);
    SECTION("same alternative")
    {
      V a = 12;
      V b = 34;
      a = std::move(b);
      CHECK(pf::get<0>(a) == 34);
    }
    SECTION("different alternative")
    {
      V a = NonTriviallyMoveAssignable{};
      V b = 42;
      a = std::move(b);
      CHECK(pf::get<0>(a) == 42);
    }
  }
}

TEST_CASE("variant special case for copy assignment")
{
  struct NeedExplicitCopyConstruction {
    NeedExplicitCopyConstruction() noexcept = default;
    explicit NeedExplicitCopyConstruction(NeedExplicitCopyConstruction const&) noexcept(false) = default;
    NeedExplicitCopyConstruction(NeedExplicitCopyConstruction&&) noexcept(true) = default;
    NeedExplicitCopyConstruction& operator=(NeedExplicitCopyConstruction const&) = default;
    NeedExplicitCopyConstruction& operator=(NeedExplicitCopyConstruction&&) = default;
  };
  pf::variant<int, NeedExplicitCopyConstruction> a = 42;
  pf::variant<int, NeedExplicitCopyConstruction> b = NeedExplicitCopyConstruction{};
  a = b;
}

TEST_CASE("variant generic assignment")
{
  SECTION("trivial case")
  {
    SECTION("same alternative")
    {
      pf::variant<int, double> a;
      a = 42;
      CHECK(pf::get<0>(a) == 42);
    }
    SECTION("different alternative")
    {
      pf::variant<int, double> a;
      a = 3.14;
      CHECK(pf::get<1>(a) == 3.14);
    }
  }
  SECTION("non-trivial case")
  {
    using V = pf::variant<int, double, NonTriviallyDestructible>;
    SECTION("same alternative")
    {
      V a = 10;
      a = 42;
      CHECK(a.index() == 0);
      CHECK(pf::get<0>(a) == 42);
    }
    SECTION("different alternative")
    {
      V a = 10;
      a = 3.14;
      CHECK(a.index() == 1);
      CHECK(pf::get<1>(a) == 3.14);
    }
  }
}

TEST_CASE("variant_detail::exactly_once")
{
  namespace vd = pf::variant_detail;

  // type not present at all -> false
  STATIC_REQUIRE(!vd::exactly_once<int>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, double>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, double, char>::value);

  // type present exactly once -> true
  STATIC_REQUIRE(vd::exactly_once<int, int>::value);
  STATIC_REQUIRE(vd::exactly_once<int, double, int>::value);
  STATIC_REQUIRE(vd::exactly_once<int, int, double>::value);
  STATIC_REQUIRE(vd::exactly_once<int, double, char, int>::value);
  STATIC_REQUIRE(vd::exactly_once<int, double, int, char>::value);

  // type present more than once -> false
  STATIC_REQUIRE(!vd::exactly_once<int, int, int>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, int, double, int>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, double, int, int>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, int, int, int>::value);

  // cv-qualified types are distinct
  STATIC_REQUIRE(vd::exactly_once<int, int, int const>::value);
  STATIC_REQUIRE(vd::exactly_once<int const, int, int const>::value);
  STATIC_REQUIRE(!vd::exactly_once<int, int const, int volatile>::value);

  // reference types are distinct
  STATIC_REQUIRE(vd::exactly_once<int, int, int&>::value);
}

TEST_CASE("variant_detail::find_index")
{
  namespace vd = pf::variant_detail;

  // single-element pack
  STATIC_REQUIRE(vd::find_index<int, int>::value == 0);

  // first element
  STATIC_REQUIRE(vd::find_index<int, int, double>::value == 0);

  // second element
  STATIC_REQUIRE(vd::find_index<double, int, double>::value == 1);

  // later position
  STATIC_REQUIRE(vd::find_index<char, int, double, char>::value == 2);

  // finds first occurrence when duplicates exist
  STATIC_REQUIRE(vd::find_index<int, int, double, int>::value == 0);
  STATIC_REQUIRE(vd::find_index<double, int, double, double>::value == 1);

  // cv-qualified types
  STATIC_REQUIRE(vd::find_index<int const, int, int const>::value == 1);
}

TEST_CASE("variant noexcept")
{
  struct NothrowAll {
    NothrowAll() noexcept = default;
    NothrowAll(NothrowAll const&) noexcept = default;
    NothrowAll(NothrowAll&&) noexcept = default;
    NothrowAll& operator=(NothrowAll const&) noexcept = default;
    NothrowAll& operator=(NothrowAll&&) noexcept = default;
  };

  struct ThrowingCopy {
    ThrowingCopy() noexcept = default;
    ThrowingCopy(ThrowingCopy const&) noexcept(false) {}
    ThrowingCopy(ThrowingCopy&&) noexcept = default;
    ThrowingCopy& operator=(ThrowingCopy const&) noexcept(false) { return *this; }
    ThrowingCopy& operator=(ThrowingCopy&&) noexcept = default;
  };

  struct ThrowingMove {
    ThrowingMove() noexcept = default;
    ThrowingMove(ThrowingMove const&) noexcept = default;
    ThrowingMove(ThrowingMove&&) noexcept(false) {}
    ThrowingMove& operator=(ThrowingMove const&) noexcept = default;
    ThrowingMove& operator=(ThrowingMove&&) noexcept(false) { return *this; }
  };

  SECTION("default construction")
  {
    STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::variant<int>>::value);
    STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::variant<int, double>>::value);
    STATIC_REQUIRE(std::is_nothrow_default_constructible<pf::variant<NothrowAll>>::value);
  }

  SECTION("copy construction")
  {
    STATIC_REQUIRE(std::is_nothrow_copy_constructible<pf::variant<int, double>>::value);
    STATIC_REQUIRE(std::is_nothrow_copy_constructible<pf::variant<int, NothrowAll>>::value);
    STATIC_REQUIRE(!std::is_nothrow_copy_constructible<pf::variant<int, ThrowingCopy>>::value);
    STATIC_REQUIRE(std::is_nothrow_copy_constructible<pf::variant<int, ThrowingMove>>::value);
  }

  SECTION("move construction")
  {
    STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::variant<int, double>>::value);
    STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::variant<int, NothrowAll>>::value);
    STATIC_REQUIRE(std::is_nothrow_move_constructible<pf::variant<int, ThrowingCopy>>::value);
    STATIC_REQUIRE(!std::is_nothrow_move_constructible<pf::variant<int, ThrowingMove>>::value);
  }

  SECTION("copy assignment")
  {
    STATIC_REQUIRE(std::is_nothrow_copy_assignable<pf::variant<int, double>>::value);
    STATIC_REQUIRE(std::is_nothrow_copy_assignable<pf::variant<int, NothrowAll>>::value);
    STATIC_REQUIRE(!std::is_nothrow_copy_assignable<pf::variant<int, ThrowingCopy>>::value);
    STATIC_REQUIRE(std::is_nothrow_copy_assignable<pf::variant<int, ThrowingMove>>::value);
  }

  SECTION("move assignment")
  {
    STATIC_REQUIRE(std::is_nothrow_move_assignable<pf::variant<int, double>>::value);
    STATIC_REQUIRE(std::is_nothrow_move_assignable<pf::variant<int, NothrowAll>>::value);
    STATIC_REQUIRE(std::is_nothrow_move_assignable<pf::variant<int, ThrowingCopy>>::value);
    STATIC_REQUIRE(!std::is_nothrow_move_assignable<pf::variant<int, ThrowingMove>>::value);
  }

  SECTION("in-place construction")
  {
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, double>, pf::in_place_index_t<0>, int>::value);
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, double>, pf::in_place_index_t<1>, double>::value);
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, ThrowingCopy>, pf::in_place_index_t<0>, int>::value);
    // ThrowingCopy's default constructor is nothrow
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, ThrowingCopy>, pf::in_place_index_t<1>>::value);
  }

  SECTION("generic construction")
  {
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, double>, int>::value);
    STATIC_REQUIRE(std::is_nothrow_constructible<pf::variant<int, double>, double>::value);

    struct ThrowingFromInt {
      ThrowingFromInt() noexcept = default;
      ThrowingFromInt(int) noexcept(false) {}
    };
    STATIC_REQUIRE(!std::is_nothrow_constructible<pf::variant<ThrowingFromInt>, int>::value);
  }

  SECTION("generic assignment")
  {
    STATIC_REQUIRE(std::is_nothrow_assignable<pf::variant<int, double>&, int>::value);
    STATIC_REQUIRE(std::is_nothrow_assignable<pf::variant<int, double>&, double>::value);

    // not nothrow when construction from the source type throws
    struct ThrowingFromInt {
      ThrowingFromInt() noexcept = default;
      ThrowingFromInt(int) noexcept(false) {}
      ThrowingFromInt& operator=(int) noexcept { return *this; }
    };
    STATIC_REQUIRE(!std::is_nothrow_assignable<pf::variant<ThrowingFromInt>&, int>::value);

    // not nothrow when assignment from the source type throws
    struct ThrowingAssignFromInt {
      ThrowingAssignFromInt() noexcept = default;
      ThrowingAssignFromInt(int) noexcept {}
      ThrowingAssignFromInt& operator=(int) noexcept(false) { return *this; }
    };
    STATIC_REQUIRE(!std::is_nothrow_assignable<pf::variant<ThrowingAssignFromInt>&, int>::value);
  }
}
