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
  ThrowsOnConstruction(ThrowsOnConstruction const&) noexcept {}
  ThrowsOnConstruction(ThrowsOnConstruction&&) noexcept {}
  ThrowsOnConstruction& operator=(ThrowsOnConstruction const&) noexcept { return *this; }
  ThrowsOnConstruction& operator=(ThrowsOnConstruction&&) noexcept { return *this; }
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

// Trivially copyable types with restricted special member functions.
// Each type has exactly one eligible trivial copy/move operation.
// Used to verify that the library selects the correct trivial operation.

struct OnlyTrivialMoveCtor {
  int value;
  OnlyTrivialMoveCtor(int v) : value(v) {}
  OnlyTrivialMoveCtor(OnlyTrivialMoveCtor const&) = delete;
  OnlyTrivialMoveCtor(OnlyTrivialMoveCtor&&) = default;
  OnlyTrivialMoveCtor& operator=(OnlyTrivialMoveCtor const&) = delete;
  OnlyTrivialMoveCtor& operator=(OnlyTrivialMoveCtor&&) = delete;
};
static_assert(std::is_trivially_copyable<OnlyTrivialMoveCtor>::value, "");

struct OnlyTrivialCopyCtor {
  int value;
  OnlyTrivialCopyCtor(int v) : value(v) {}
  OnlyTrivialCopyCtor(OnlyTrivialCopyCtor const&) = default;
  OnlyTrivialCopyCtor(OnlyTrivialCopyCtor&&) = delete;
  OnlyTrivialCopyCtor& operator=(OnlyTrivialCopyCtor const&) = delete;
  OnlyTrivialCopyCtor& operator=(OnlyTrivialCopyCtor&&) = delete;
};
static_assert(std::is_trivially_copyable<OnlyTrivialCopyCtor>::value, "");

struct OnlyTrivialMoveAssign {
  int value;
  OnlyTrivialMoveAssign(int v) : value(v) {}
  OnlyTrivialMoveAssign(OnlyTrivialMoveAssign const&) = delete;
  OnlyTrivialMoveAssign(OnlyTrivialMoveAssign&&) = delete;
  OnlyTrivialMoveAssign& operator=(OnlyTrivialMoveAssign const&) = delete;
  OnlyTrivialMoveAssign& operator=(OnlyTrivialMoveAssign&&) = default;
};
static_assert(std::is_trivially_copyable<OnlyTrivialMoveAssign>::value, "");

struct OnlyTrivialCopyAssign {
  int value;
  OnlyTrivialCopyAssign(int v) : value(v) {}
  OnlyTrivialCopyAssign(OnlyTrivialCopyAssign const&) = delete;
  OnlyTrivialCopyAssign(OnlyTrivialCopyAssign&&) = delete;
  OnlyTrivialCopyAssign& operator=(OnlyTrivialCopyAssign const&) = default;
  OnlyTrivialCopyAssign& operator=(OnlyTrivialCopyAssign&&) = delete;
};
static_assert(std::is_trivially_copyable<OnlyTrivialCopyAssign>::value, "");

TEST_CASE("never_valueless check")
{
  STATIC_REQUIRE(pf::variant_detail::make_variadic_union<int, double>::type::never_valueless);
  STATIC_REQUIRE(!pf::variant_detail::make_variadic_union<int, ThrowsOnConstruction>::type::never_valueless);
}

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

TEST_CASE("variant in-place type construction")
{
  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_type_t<int>, int>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<int, double>, pf::in_place_type_t<double>, double>::value);

  STATIC_REQUIRE(!std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_type_t<NotDefaultConstructible>>::value);
  STATIC_REQUIRE(std::is_constructible<pf::variant<NotDefaultConstructible>, pf::in_place_type_t<NotDefaultConstructible>, int>::value);

  // duplicate type -> should not be constructible
  STATIC_REQUIRE(!std::is_constructible<pf::variant<int, int>, pf::in_place_type_t<int>, int>::value);

  {
    pf::variant<int, double> v(pf::in_place_type_t<int>{}, 42);
    CHECK(v.index() == 0);
    CHECK(pf::get<int>(v) == 42);
  }
  {
    pf::variant<int, double> v(pf::in_place_type_t<double>{}, 3.14);
    CHECK(v.index() == 1);
    CHECK(pf::get<double>(v) == 3.14);
  }
}

TEST_CASE("variant emplace by type")
{
  SECTION("same alternative")
  {
    pf::variant<int, double> v = 42;
    auto& ref = v.emplace<int>(99);
    CHECK(v.index() == 0);
    CHECK(pf::get<int>(v) == 99);
    CHECK(ref == 99);
  }
  SECTION("different alternative")
  {
    pf::variant<int, double> v = 42;
    auto& ref = v.emplace<double>(3.14);
    CHECK(v.index() == 1);
    CHECK(pf::get<double>(v) == 3.14);
    CHECK(ref == 3.14);
  }
  SECTION("non-trivially destructible")
  {
    pf::variant<int, NonTriviallyDestructible> v = 42;
    v.emplace<NonTriviallyDestructible>();
    CHECK(v.index() == 1);
    v.emplace<int>(10);
    CHECK(v.index() == 0);
    CHECK(pf::get<int>(v) == 10);
  }
  SECTION("from valueless")
  {
    pf::variant<int, ThrowsOnConstruction> v = 42;
    make_valueless(v);
    CHECK(v.valueless_by_exception());
    v.emplace<int>(7);
    CHECK(!v.valueless_by_exception());
    CHECK(v.index() == 0);
    CHECK(pf::get<int>(v) == 7);
  }
}

TEST_CASE("variant holds_alternative")
{
  {
    pf::variant<int, double> v = 42;
    CHECK(pf::holds_alternative<int>(v));
    CHECK(!pf::holds_alternative<double>(v));
  }
  {
    pf::variant<int, double> v = 3.14;
    CHECK(!pf::holds_alternative<int>(v));
    CHECK(pf::holds_alternative<double>(v));
  }
  {
    pf::variant<int, ThrowsOnConstruction> v = 42;
    make_valueless(v);
    CHECK(!pf::holds_alternative<int>(v));
    CHECK(!pf::holds_alternative<ThrowsOnConstruction>(v));
  }
}

TEST_CASE("variant get by type")
{
  SECTION("lvalue ref")
  {
    pf::variant<int, double> v = 42;
    CHECK(pf::get<int>(v) == 42);
    CHECK_THROWS(pf::get<double>(v));
  }
  SECTION("const lvalue ref")
  {
    pf::variant<int, double> const v = 3.14;
    CHECK(pf::get<double>(v) == 3.14);
    CHECK_THROWS(pf::get<int>(v));
  }
  SECTION("rvalue ref")
  {
    pf::variant<int, double> v = 42;
    CHECK(pf::get<int>(std::move(v)) == 42);
  }
  SECTION("const rvalue ref")
  {
    pf::variant<int, double> const v = 3.14;
    CHECK(pf::get<double>(std::move(v)) == 3.14);
  }
  SECTION("return type correctness")
  {
    using V = pf::variant<int, double>;
    STATIC_REQUIRE(std::is_same<decltype(pf::get<int>(std::declval<V&>())), int&>::value);
    STATIC_REQUIRE(std::is_same<decltype(pf::get<int>(std::declval<V const&>())), int const&>::value);
    STATIC_REQUIRE(std::is_same<decltype(pf::get<int>(std::declval<V&&>())), int&&>::value);
    STATIC_REQUIRE(std::is_same<decltype(pf::get<int>(std::declval<V const&&>())), int const&&>::value);
  }
}

TEST_CASE("variant get_if by index")
{
  SECTION("non-null matching")
  {
    pf::variant<int, double> v = 42;
    auto* p = pf::get_if<0>(&v);
    STATIC_REQUIRE(std::is_same<decltype(p), int*>::value);
    CHECK(p != nullptr);
    CHECK(*p == 42);
  }
  SECTION("non-null non-matching")
  {
    pf::variant<int, double> v = 42;
    auto* p = pf::get_if<1>(&v);
    CHECK(p == nullptr);
  }
  SECTION("null pointer")
  {
    pf::variant<int, double>* v = nullptr;
    auto* p = pf::get_if<0>(v);
    CHECK(p == nullptr);
  }
  SECTION("const variant")
  {
    pf::variant<int, double> const v = 3.14;
    auto* p = pf::get_if<1>(&v);
    STATIC_REQUIRE(std::is_same<decltype(p), double const*>::value);
    CHECK(p != nullptr);
    CHECK(*p == 3.14);
  }
}

TEST_CASE("variant get_if by type")
{
  SECTION("non-null matching")
  {
    pf::variant<int, double> v = 42;
    auto* p = pf::get_if<int>(&v);
    STATIC_REQUIRE(std::is_same<decltype(p), int*>::value);
    CHECK(p != nullptr);
    CHECK(*p == 42);
  }
  SECTION("non-null non-matching")
  {
    pf::variant<int, double> v = 42;
    auto* p = pf::get_if<double>(&v);
    CHECK(p == nullptr);
  }
  SECTION("null pointer")
  {
    pf::variant<int, double>* v = nullptr;
    auto* p = pf::get_if<int>(v);
    CHECK(p == nullptr);
  }
  SECTION("const variant")
  {
    pf::variant<int, double> const v = 3.14;
    auto* p = pf::get_if<double>(&v);
    STATIC_REQUIRE(std::is_same<decltype(p), double const*>::value);
    CHECK(p != nullptr);
    CHECK(*p == 3.14);
  }
  SECTION("valueless")
  {
    pf::variant<int, ThrowsOnConstruction> v = 42;
    make_valueless(v);
    CHECK(pf::get_if<int>(&v) == nullptr);
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

#if __cplusplus >= 201402L
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
#endif

TEST_CASE("variant visit")
{
  SECTION("lvalue variant")
  {
    pf::variant<int, double> v = 42;
    struct visitor {
      int operator()(int i) const { return i * 2; }
      int operator()(double d) const { return static_cast<int>(d); }
    };
    CHECK(pf::visit(visitor{}, v) == 84);
  }
  SECTION("const lvalue variant")
  {
    pf::variant<int, double> const v = 3.14;
    struct visitor {
      double operator()(int i) const { return static_cast<double>(i); }
      double operator()(double d) const { return d * 2; }
    };
    CHECK(pf::visit(visitor{}, v) == 6.28);
  }
  SECTION("rvalue variant")
  {
    pf::variant<int, double> v = 42;
    struct visitor {
      int operator()(int&& i) const { return i + 1; }
      int operator()(double&& d) const { return static_cast<int>(d); }
    };
    CHECK(pf::visit(visitor{}, std::move(v)) == 43);
  }
  SECTION("valueless variant throws")
  {
    pf::variant<int, ThrowsOnConstruction> v = 42;
    make_valueless(v);
    struct visitor {
      int operator()(int) const { return 0; }
      int operator()(ThrowsOnConstruction const&) const { return 1; }
    };
    CHECK_THROWS_AS(pf::visit(visitor{}, v), pf::bad_variant_access);
  }
  SECTION("void return type")
  {
    pf::variant<int, double> v = 42;
    int result = 0;
    struct visitor {
      int& result_ref;
      void operator()(int i) const { result_ref = i; }
      void operator()(double) const {}
    };
    pf::visit(visitor{result}, v);
    CHECK(result == 42);
  }
}

TEST_CASE("variant multi-visit")
{
  SECTION("two variants")
  {
    pf::variant<int, double> v1 = 42;
    pf::variant<char, float> v2 = 'a';
    struct visitor {
      int operator()(int i, char c) const { return i + c; }
      int operator()(int i, float) const { return i; }
      int operator()(double, char c) const { return c; }
      int operator()(double, float) const { return 0; }
    };
    CHECK(pf::visit(visitor{}, v1, v2) == 42 + 'a');
  }
  SECTION("two variants - second alternative")
  {
    pf::variant<int, double> v1 = 3.14;
    pf::variant<char, float> v2 = 1.5f;
    struct visitor {
      int operator()(int, char) const { return 1; }
      int operator()(int, float) const { return 2; }
      int operator()(double, char) const { return 3; }
      int operator()(double, float) const { return 4; }
    };
    CHECK(pf::visit(visitor{}, v1, v2) == 4);
  }
  SECTION("three variants")
  {
    pf::variant<int, double> v1 = 42;
    pf::variant<char, float> v2 = 'b';
    pf::variant<long, short> v3(pf::in_place_index_t<1>{}, static_cast<short>(10));
    struct visitor {
      int operator()(int i, char c, long) const { return i + c + 100; }
      int operator()(int i, char c, short s) const { return i + c + s; }
      int operator()(int, float, long) const { return 0; }
      int operator()(int, float, short) const { return 0; }
      int operator()(double, char, long) const { return 0; }
      int operator()(double, char, short) const { return 0; }
      int operator()(double, float, long) const { return 0; }
      int operator()(double, float, short) const { return 0; }
    };
    CHECK(pf::visit(visitor{}, v1, v2, v3) == 42 + 'b' + 10);
  }
  SECTION("valueless variant throws")
  {
    pf::variant<int, ThrowsOnConstruction> v1 = 42;
    pf::variant<int, double> v2 = 3.14;
    make_valueless(v1);
    struct visitor {
      int operator()(int, int) const { return 0; }
      int operator()(int, double) const { return 1; }
      int operator()(ThrowsOnConstruction const&, int) const { return 2; }
      int operator()(ThrowsOnConstruction const&, double) const { return 3; }
    };
    CHECK_THROWS_AS(pf::visit(visitor{}, v1, v2), pf::bad_variant_access);
  }
  SECTION("void return type")
  {
    pf::variant<int, double> v1 = 42;
    pf::variant<char, float> v2 = 'x';
    int result = 0;
    struct visitor {
      int& result_ref;
      void operator()(int i, char c) const { result_ref = i + c; }
      void operator()(int, float) const {}
      void operator()(double, char) const {}
      void operator()(double, float) const {}
    };
    pf::visit(visitor{result}, v1, v2);
    CHECK(result == 42 + 'x');
  }
  SECTION("mixed ref-qualifiers")
  {
    pf::variant<int, double> v1 = 42;
    pf::variant<char, float> const v2 = 'a';
    struct visitor {
      int operator()(int i, char c) const { return i + c; }
      int operator()(int, float) const { return 0; }
      int operator()(double, char) const { return 0; }
      int operator()(double, float) const { return 0; }
    };
    CHECK(pf::visit(visitor{}, v1, v2) == 42 + 'a');
  }
}

TEST_CASE("variant swap")
{
  SECTION("same alternative")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 99;
    a.swap(b);
    CHECK(pf::get<0>(a) == 99);
    CHECK(pf::get<0>(b) == 42);
  }
  SECTION("different alternatives")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 3.14;
    a.swap(b);
    CHECK(a.index() == 1);
    CHECK(pf::get<1>(a) == 3.14);
    CHECK(b.index() == 0);
    CHECK(pf::get<0>(b) == 42);
  }
  SECTION("free function swap")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 3.14;
    using std::swap;
    swap(a, b);
    CHECK(a.index() == 1);
    CHECK(pf::get<1>(a) == 3.14);
    CHECK(b.index() == 0);
    CHECK(pf::get<0>(b) == 42);
  }
  SECTION("both valueless")
  {
    pf::variant<int, ThrowsOnConstruction> a = 42;
    pf::variant<int, ThrowsOnConstruction> b = 42;
    make_valueless(a);
    make_valueless(b);
    a.swap(b);
    CHECK(a.valueless_by_exception());
    CHECK(b.valueless_by_exception());
  }
}

TEST_CASE("monostate comparisons")
{
  pf::monostate a, b;
  CHECK(a == b);
  CHECK_FALSE(a != b);
  CHECK_FALSE(a < b);
  CHECK(a <= b);
  CHECK_FALSE(a > b);
  CHECK(a >= b);
}

TEST_CASE("variant comparison operators")
{
  SECTION("equal - same alternative same value")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 42;
    CHECK(a == b);
    CHECK_FALSE(a != b);
  }
  SECTION("equal - same alternative different value")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 99;
    CHECK_FALSE(a == b);
    CHECK(a != b);
  }
  SECTION("different alternatives")
  {
    pf::variant<int, double> a = 42;
    pf::variant<int, double> b = 3.14;
    CHECK_FALSE(a == b);
    CHECK(a != b);
    CHECK(a < b);
    CHECK(a <= b);
    CHECK_FALSE(a > b);
    CHECK_FALSE(a >= b);
  }
  SECTION("ordering - same alternative")
  {
    pf::variant<int, double> a = 10;
    pf::variant<int, double> b = 42;
    CHECK(a < b);
    CHECK(a <= b);
    CHECK_FALSE(a > b);
    CHECK_FALSE(a >= b);
    CHECK(b > a);
    CHECK(b >= a);
  }
  SECTION("monostate variant comparison")
  {
    pf::variant<pf::monostate, int> a;
    pf::variant<pf::monostate, int> b;
    CHECK(a == b);
    CHECK(a <= b);
    CHECK(a >= b);
  }
}

TEST_CASE("trivially copyable variant emplace (same index)")
{
  SECTION("only trivial move ctor")
  {
    pf::variant<OnlyTrivialMoveCtor> v(pf::in_place_index_t<0>{}, 1);
    CHECK(pf::get<0>(v).value == 1);
    v.emplace<0>(2);
    CHECK(pf::get<0>(v).value == 2);
  }
  SECTION("only trivial copy ctor")
  {
    pf::variant<OnlyTrivialCopyCtor> v(pf::in_place_index_t<0>{}, 1);
    CHECK(pf::get<0>(v).value == 1);
    v.emplace<0>(2);
    CHECK(pf::get<0>(v).value == 2);
  }
  SECTION("only trivial move assign")
  {
    pf::variant<OnlyTrivialMoveAssign> v(pf::in_place_index_t<0>{}, 1);
    CHECK(pf::get<0>(v).value == 1);
    v.emplace<0>(2);
    CHECK(pf::get<0>(v).value == 2);
  }
  SECTION("only trivial copy assign")
  {
    pf::variant<OnlyTrivialCopyAssign> v(pf::in_place_index_t<0>{}, 1);
    CHECK(pf::get<0>(v).value == 1);
    v.emplace<0>(2);
    CHECK(pf::get<0>(v).value == 2);
  }
}

TEST_CASE("trivially copyable variant emplace (different index)")
{
  SECTION("only trivial move ctor")
  {
    pf::variant<int, OnlyTrivialMoveCtor> v(pf::in_place_index_t<0>{}, 0);
    CHECK(v.index() == 0);
    v.emplace<1>(42);
    CHECK(v.index() == 1);
    CHECK(pf::get<1>(v).value == 42);
  }
  SECTION("only trivial copy ctor")
  {
    pf::variant<int, OnlyTrivialCopyCtor> v(pf::in_place_index_t<0>{}, 0);
    CHECK(v.index() == 0);
    v.emplace<1>(42);
    CHECK(v.index() == 1);
    CHECK(pf::get<1>(v).value == 42);
  }
  SECTION("only trivial move assign")
  {
    pf::variant<int, OnlyTrivialMoveAssign> v(pf::in_place_index_t<0>{}, 0);
    CHECK(v.index() == 0);
    v.emplace<1>(42);
    CHECK(v.index() == 1);
    CHECK(pf::get<1>(v).value == 42);
  }
  SECTION("only trivial copy assign")
  {
    pf::variant<int, OnlyTrivialCopyAssign> v(pf::in_place_index_t<0>{}, 0);
    CHECK(v.index() == 0);
    v.emplace<1>(42);
    CHECK(v.index() == 1);
    CHECK(pf::get<1>(v).value == 42);
  }
}

