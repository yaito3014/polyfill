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

  // dereference
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int>&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int> const&>()), int const&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int>&&>()), int&&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int> const&&>()), int const&&>::value);

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
    CHECK(a.has_value());
    CHECK(*a == 3);
  }

  // generic copy assignment
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = 3.14;
    b = a;
    CHECK(b.has_value());
    CHECK(*b == 42.0);
  }

  // generic move assignment
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = 3.14;
    b = std::move(a);
    CHECK(b.has_value());
    CHECK(*b == 42.0);
  }

  // emplace
  {
    pf::optional<int> a = 42;
    a.emplace(33 - 4);
    CHECK(a.has_value());
    CHECK(*a == 29);
  }

  // swap
  {
    {
      pf::optional<int> a = 33;
      pf::optional<int> b = 4;
      a.swap(b);
      CHECK(a.has_value());
      CHECK(b.has_value());
      CHECK(*a == 4);
      CHECK(*b == 33);
    }
    {
      pf::optional<int> a = 42;
      pf::optional<int> b;
      a.swap(b);
      CHECK(!a.has_value());
      CHECK(b.has_value());
      CHECK(*b == 42);
    }
    {
      pf::optional<int> a;
      pf::optional<int> b = 42;
      a.swap(b);
      CHECK(a.has_value());
      CHECK(!b.has_value());
      CHECK(*a == 42);
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
    CHECK(s.has_value());
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

  // and_then
  {
    using OI = pf::optional<int>;
    using OD = pf::optional<double>;

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&>().and_then(std::declval<OI (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&>().and_then(std::declval<OI (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&&>().and_then(std::declval<OI (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&&>().and_then(std::declval<OI (&)(int)>())), OI>::value);

    {
      auto const identity = [](int x) -> OI { return x; };
      auto const ret_null = [](int) -> OI { return pf::nullopt_holder::value; };
      CHECK(OI{42}.and_then(identity).value() == 42);
      CHECK(!OI{42}.and_then(ret_null).has_value());
      CHECK(!OI{pf::nullopt_holder::value}.and_then(identity).has_value());
      CHECK(!OI{pf::nullopt_holder::value}.and_then(ret_null).has_value());
    }

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&>().and_then(std::declval<OD (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&>().and_then(std::declval<OD (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&&>().and_then(std::declval<OD (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&&>().and_then(std::declval<OD (&)(int)>())), OD>::value);

    {
      auto const convert = [](int x) -> OD { return static_cast<double>(x); };
      auto const ret_null = [](int) -> OD { return pf::nullopt_holder::value; };
      CHECK(OI{42}.and_then(convert).value() == 42.);
      CHECK(!OI{42}.and_then(ret_null).has_value());
      CHECK(!OI{pf::nullopt_holder::value}.and_then(convert).has_value());
      CHECK(!OI{pf::nullopt_holder::value}.and_then(ret_null).has_value());
    }
  }

  // transform
  {
    using OI = pf::optional<int>;
    using OD = pf::optional<double>;

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&>().transform(std::declval<int (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&>().transform(std::declval<int (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&&>().transform(std::declval<int (&)(int)>())), OI>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&&>().transform(std::declval<int (&)(int)>())), OI>::value);

    {
      auto const identity = [](int x) -> int { return x; };
      CHECK(OI{42}.transform(identity).value() == 42);
      CHECK(!OI{pf::nullopt_holder::value}.transform(identity).has_value());
    }
    
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&>().transform(std::declval<double (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&>().transform(std::declval<double (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI&&>().transform(std::declval<double (&)(int)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OI const&&>().transform(std::declval<double (&)(int)>())), OD>::value);

    {
      auto const convert = [](int x) -> int { return x; };
      CHECK(OI{42}.transform(convert).value() == 42.);
      CHECK(!OI{pf::nullopt_holder::value}.transform(convert).has_value());
    }
  }

  // TODO: add non trivial tests
}

TEST_CASE("ref optional")
{
  struct Base {};
  struct Derived : Base {};

  // dereference
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int&>&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int&> const&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int&>&&>()), int&>::value);
  STATIC_REQUIRE(std::is_same<decltype(*std::declval<pf::optional<int&> const&&>()), int&>::value);

  // default construction
  {
    pf::optional<int&> opt;
    CHECK(!opt.has_value());
  }

  // in-place construction
  {
    int x = 42;
    pf::optional<int&> opt(pf::in_place_holder::value, x);
    CHECK(opt.has_value());
    CHECK(std::addressof(*opt) == std::addressof(x));
  }

  // generic construction
  {
    Derived x;
    pf::optional<Base&> opt = x;
    CHECK(opt.has_value());
    CHECK(std::addressof(*opt) == std::addressof(x));
  }

  // generic copy construction
  {
    Derived x;
    pf::optional<Derived&> a = x;
    pf::optional<Base&> b = a;
    CHECK(b.has_value());
    CHECK(std::addressof(*a) == std::addressof(*b));
  }

  // generic move construction
  {
    Derived x;
    pf::optional<Derived&> a = x;
    pf::optional<Base&> b = std::move(a);
    CHECK(b.has_value());
    CHECK(std::addressof(*b) == std::addressof(x));
  }

  // nullopt assignment
  {
    int x = 42;
    pf::optional<int&> opt = x;
    opt = pf::nullopt_holder::value;
    CHECK(!opt.has_value());
  }

  // generic assignment
  {
    Derived x;
    pf::optional<Base&> opt;
    opt = x;
    CHECK(opt.has_value());
    CHECK(std::addressof(*opt) == std::addressof(x));
  }

  // generic copy assignment
  {
    Derived x;
    pf::optional<Derived&> a = x;
    pf::optional<Base&> b;
    b = a;
    CHECK(b.has_value());
    CHECK(std::addressof(*a) == std::addressof(*b));
  }

  // generic move assignment
  {
    Derived x;
    pf::optional<Derived&> a = x;
    pf::optional<Base&> b;
    b = std::move(a);
    CHECK(b.has_value());
    CHECK(std::addressof(*b) == std::addressof(x));
  }

  // emplace
  {
    int x = 12;
    int y = 34;
    pf::optional<int&> opt = x;
    opt.emplace(y);
    CHECK(std::addressof(*opt) == std::addressof(y));
  }

  // swap
  {
    int x = 12;
    int y = 34;
    {
      pf::optional<int&> a = x;
      pf::optional<int&> b = y;
      a.swap(b);
      CHECK(a.has_value());
      CHECK(b.has_value());
      CHECK(std::addressof(*a) == std::addressof(y));
      CHECK(std::addressof(*b) == std::addressof(x));
    }
    {
      pf::optional<int&> a = x;
      pf::optional<int&> b;
      a.swap(b);
      CHECK(!a.has_value());
      CHECK(b.has_value());
      CHECK(std::addressof(*b) == std::addressof(x));
    }
    {
      pf::optional<int&> a;
      pf::optional<int&> b = y;
      a.swap(b);
      CHECK(a.has_value());
      CHECK(!b.has_value());
      CHECK(std::addressof(*a) == std::addressof(y));
    }
    {
      pf::optional<int&> a;
      pf::optional<int&> b;
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

    S s{42};
    pf::optional<S&> opt = s;
    CHECK(opt.has_value());
    CHECK(opt->x == 42);
    CHECK(pf::as_const(opt)->x == 42);
  }

  // checked value access
  {
    int x = 42;
    pf::optional<int&> a = x;
    CHECK(std::addressof(a.value()) == std::addressof(x));
    pf::optional<int&> b;
    CHECK_THROWS_AS(b.value(), pf::bad_optional_access);

    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int&>&>().value()), int&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int&> const&>().value()), int&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int&>&&>().value()), int&>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<pf::optional<int&> const&&>().value()), int&>::value);
  }

  // fallback value access
  {
    pf::optional<int&> opt;
    CHECK(opt.value_or(42) == 42);
    int x = 12;
    opt.emplace(x);
    CHECK(opt.value_or(42) == x);
  }

  // and_then
  {
    using OB = pf::optional<Base&>;
    using OD = pf::optional<Derived&>;

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&>().and_then(std::declval<OD (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&>().and_then(std::declval<OD (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&&>().and_then(std::declval<OD (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&&>().and_then(std::declval<OD (&)(Derived&)>())), OD>::value);

    {
      auto const identity = [](Derived& d) -> OD { return d; };
      auto const ret_null = [](Derived&) -> OD { return pf::nullopt_holder::value; };
      Derived x;
      CHECK(std::addressof(OD{x}.and_then(identity).value()) == std::addressof(x));
      CHECK(!OD{x}.and_then(ret_null).has_value());
      CHECK(!OD{pf::nullopt_holder::value}.and_then(identity).has_value());
      CHECK(!OD{pf::nullopt_holder::value}.and_then(ret_null).has_value());
    }

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&>().and_then(std::declval<OB (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&>().and_then(std::declval<OB (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&&>().and_then(std::declval<OB (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&&>().and_then(std::declval<OB (&)(Derived&)>())), OB>::value);

    {
      auto const convert = [](Derived& d) -> OB { return d; };
      auto const ret_null = [](Derived&) -> OB { return pf::nullopt_holder::value; };
      Derived x;
      CHECK(std::addressof(OD{x}.and_then(convert).value()) == std::addressof(x));
      CHECK(!OD{x}.and_then(ret_null).has_value());
      CHECK(!OD{pf::nullopt_holder::value}.and_then(convert).has_value());
      CHECK(!OD{pf::nullopt_holder::value}.and_then(ret_null).has_value());
    }
  }
  
  // transform
  {
    using OB = pf::optional<Base&>;
    using OD = pf::optional<Derived&>;

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&>().transform(std::declval<Derived& (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&>().transform(std::declval<Derived& (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&&>().transform(std::declval<Derived& (&)(Derived&)>())), OD>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&&>().transform(std::declval<Derived& (&)(Derived&)>())), OD>::value);

    {
      auto const identity = [](Derived& d) -> Derived& { return d; };
      Derived x;
      CHECK(std::addressof(OD{x}.transform(identity).value()) == std::addressof(x));
      CHECK(!OD{pf::nullopt_holder::value}.transform(identity).has_value());
    }

    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&>().transform(std::declval<Base& (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&>().transform(std::declval<Base& (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD&&>().transform(std::declval<Base& (&)(Derived&)>())), OB>::value);
    STATIC_REQUIRE(std::is_same<decltype(std::declval<OD const&&>().transform(std::declval<Base& (&)(Derived&)>())), OB>::value);

    {
      auto const convert = [](Derived& d) -> Base& { return d; };
      Derived x;
      CHECK(std::addressof(OD{x}.transform(convert).value()) == std::addressof(x));
      CHECK(!OD{pf::nullopt_holder::value}.transform(convert).has_value());
    }
  }
}
