#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/optional.hpp>
#include <yk/polyfill/utility.hpp>

#include <type_traits>
#include <utility>

namespace pf = yk::polyfill;

struct NeedExplicitConvertion {
  int value;
  explicit NeedExplicitConvertion(int arg) : value(arg) {}
};

TEST_CASE("optional")
{
  // triviality check
  STATIC_REQUIRE(std::is_trivially_copy_constructible<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_constructible<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_copy_assignable<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_move_assignable<pf::optional<int>>::value);
  STATIC_REQUIRE(std::is_trivially_destructible<pf::optional<int>>::value);

  // default construction
  {
    pf::optional<int> opt;
    CHECK(!opt.has_value());
  }

  // nullopt construction
  {
    pf::optional<int> opt(pf::nullopt_holder::value);
    CHECK(!opt.has_value());
  }

  // in-place construction
  {
    pf::optional<int> opt(pf::in_place_holder::value, 42);
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }

  // generic construction
  {
    pf::optional<int> opt = 42;
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }

  // generic copy construction
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = a;
    CHECK(*b == 42.0);
  }

  // generic move construction
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = std::move(a);
    CHECK(*b == 42.0);
  }

  // boolean tolerance test
  {
    {
      pf::optional<int> a;
      pf::optional<bool> b = a;
      CHECK(!a.has_value());
      CHECK(!b.has_value());
    }

    {
      pf::optional<int> a = 42;
      pf::optional<bool> b = a;
      CHECK(b.has_value());
      CHECK(*b);
    }

    {
      pf::optional<int> a = 0;
      pf::optional<bool> b = a;
      CHECK(b.has_value());
      CHECK(!*b);
    }
  }

  // explicitation check
  {
    STATIC_REQUIRE(std::is_constructible<pf::optional<NeedExplicitConvertion>, int>::value);
    STATIC_REQUIRE(!std::is_convertible<int, pf::optional<NeedExplicitConvertion>>::value);

    STATIC_REQUIRE(std::is_constructible<pf::optional<NeedExplicitConvertion>, pf::optional<int>>::value);
    STATIC_REQUIRE(!std::is_convertible<pf::optional<int>, pf::optional<NeedExplicitConvertion>>::value);
  }

  // nullopt assignment
  {
    pf::optional<int> a = 42;
    a = pf::nullopt_holder::value;
    CHECK(!a.has_value());
  }

  // generic assignment
  {
    pf::optional<int> a = 42;
    a = 3.14;
    CHECK(*a == 3);
  }

  // generic copy assignment
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = 3.14;
    b = a;
    CHECK(*b == 42.0);
  }

  // generic move assignment
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = 3.14;
    b = std::move(a);
    CHECK(*b == 42.0);
  }

  // emplace
  {
    pf::optional<int> a = 42;
    a.emplace(33 - 4);
    CHECK(*a == 29);
  }

  // swap
  {
    {
      pf::optional<int> a = 33;
      pf::optional<int> b = 4;
      a.swap(b);
      CHECK(*a == 4);
      CHECK(*b == 33);
    }
    {
      pf::optional<int> a = 42;
      pf::optional<int> b;
      a.swap(b);
      CHECK(!a.has_value());
      CHECK(*b == 42);
    }
    {
      pf::optional<int> a;
      pf::optional<int> b = 42;
      a.swap(b);
      CHECK(*a == 42);
      CHECK(!b.has_value());
    }
    {
      pf::optional<int> a;
      pf::optional<int> b;
      a.swap(b);
      CHECK(!a.has_value());
      CHECK(!b.has_value());
    }
  }

  // member access
  {
    struct S {
      int x;
      explicit S(int arg) : x(arg) {}
    };

    pf::optional<S> s(pf::in_place_holder::value, 42);
    CHECK(s->x == 42);
    CHECK(pf::as_const(s)->x == 42);
  }

  // checked value access
  {
    pf::optional<int> opt;
    CHECK_THROWS_AS(opt.value(), pf::bad_optional_access);
    opt.emplace(42);
    CHECK_NOTHROW(opt.value() == 42);

    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int>&>().value()), int&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int> const&>().value()), int const&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int>&&>().value()), int&&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int> const&&>().value()), int const&&>::value);
  }

  // fallback value access
  {
    pf::optional<int> opt;
    CHECK(opt.value_or(42) == 42);
    opt.emplace(33);
    CHECK(opt.value_or(4) == 33);
  }

  // ref optional
  {
    int x = 42;
    pf::optional<int&> opt = x;
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }

  // TODO: add non trivial tests
}
