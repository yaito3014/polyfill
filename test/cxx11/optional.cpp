#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/optional.hpp>
#include <yk/polyfill/utility.hpp>

#include <memory>
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
      auto const convert = [](int x) -> double { return static_cast<double>(x); };
      CHECK(OI{42}.transform(convert).value() == 42.);
      CHECK(!OI{pf::nullopt_holder::value}.transform(convert).has_value());
    }
  }

  // or_else
  {
    auto const producer = []() -> pf::optional<int> { return 12; };
    auto const ret_null = []() -> pf::optional<int> { return pf::nullopt_holder::value; };

    CHECK(pf::optional<int>{34}.or_else(producer).value() == 34);
    CHECK(pf::optional<int>{34}.or_else(ret_null).value() == 34);
    CHECK(pf::optional<int>{pf::nullopt_holder::value}.or_else(producer).value() == 12);
    CHECK(!pf::optional<int>{pf::nullopt_holder::value}.or_else(ret_null).has_value());
  }

  // iterator
  {
    using It = pf::optional<int>::iterator;
#if __cpp_lib_ranges >= 201911L
    STATIC_REQUIRE(std::is_base_of<std::contiguous_iterator_tag, std::iterator_traits<It>::iterator_category>::value);
#else
    STATIC_REQUIRE(std::is_base_of<std::random_access_iterator_tag, std::iterator_traits<It>::iterator_category>::value);
#endif
    {
      pf::optional<int> opt;
      CHECK(opt.end() - opt.begin() == 0);
    }
    {
      pf::optional<int> opt = 42;
      CHECK(opt.end() - opt.begin() == 1);
      CHECK(*opt.begin() == 42);
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

  // or_else
  {
    int x = 12;
    int y = 34;

    auto const producer = [&]() -> pf::optional<int&> { return x; };
    auto const ret_null = []() -> pf::optional<int&> { return pf::nullopt_holder::value; };

    CHECK(std::addressof(pf::optional<int&>{y}.or_else(producer).value()) == std::addressof(y));
    CHECK(std::addressof(pf::optional<int&>{y}.or_else(ret_null).value()) == std::addressof(y));
    CHECK(std::addressof(pf::optional<int&>{pf::nullopt_holder::value}.or_else(producer).value()) == std::addressof(x));
    CHECK(!pf::optional<int&>{pf::nullopt_holder::value}.or_else(ret_null).has_value());
  }

  // iterator
  {
    using It = pf::optional<int&>::iterator;
#if __cpp_lib_ranges >= 201911L
    STATIC_REQUIRE(std::is_base_of<std::contiguous_iterator_tag, std::iterator_traits<It>::iterator_category>::value);
#else
    STATIC_REQUIRE(std::is_base_of<std::random_access_iterator_tag, std::iterator_traits<It>::iterator_category>::value);
#endif
    int x = 42;
    {
      pf::optional<int&> opt;
      CHECK(opt.end() - opt.begin() == 0);
    }
    {
      pf::optional<int&> opt = x;
      CHECK(opt.end() - opt.begin() == 1);
      CHECK(std::addressof(*opt.begin()) == std::addressof(x));
    }
  }
}

TEST_CASE("optional non-trivial")
{
  struct MoveOnly {
    MoveOnly() = default;
    MoveOnly(MoveOnly const&) = delete;
    MoveOnly(MoveOnly&&) = default;
  };

  pf::optional<MoveOnly> opt = MoveOnly{};

  // TODO: add more tests
}

TEST_CASE("optional noexcept propagation")
{
  // Test type with noexcept operations
  struct NoexceptOps {
    NoexceptOps() noexcept = default;
    NoexceptOps(int) noexcept {}
    NoexceptOps(NoexceptOps const&) noexcept = default;
    NoexceptOps(NoexceptOps&&) noexcept = default;
    NoexceptOps& operator=(NoexceptOps const&) noexcept = default;
    NoexceptOps& operator=(NoexceptOps&&) noexcept = default;
    ~NoexceptOps() noexcept = default;
    void swap(NoexceptOps&) noexcept {}
  };

  // Test type with potentially throwing operations
  struct ThrowingOps {
    ThrowingOps() {}
    ThrowingOps(int) {}
    ThrowingOps(ThrowingOps const&) {}
    ThrowingOps(ThrowingOps&&) {}
    ThrowingOps& operator=(ThrowingOps const&) { return *this; }
    ThrowingOps& operator=(ThrowingOps&&) { return *this; }
    ~ThrowingOps() {}
    void swap(ThrowingOps&) {}
  };

  // Construction
  {
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>()));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(pf::nullopt_holder::value)));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(pf::in_place_holder::value, 42)));

    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps>()));
    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps>(pf::nullopt_holder::value)));
    STATIC_REQUIRE(!noexcept(pf::optional<ThrowingOps>(pf::in_place_holder::value, 42)));
  }

  // Copy/Move construction
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;

    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(a)));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(std::move(a))));

    STATIC_REQUIRE(!noexcept(pf::optional<ThrowingOps>(b)));
    STATIC_REQUIRE(!noexcept(pf::optional<ThrowingOps>(std::move(b))));
  }

  // Copy/Move assignment
  {
    pf::optional<NoexceptOps> a1, a2;
    pf::optional<ThrowingOps> b1, b2;

    STATIC_REQUIRE(noexcept(a1 = a2));
    STATIC_REQUIRE(noexcept(a1 = std::move(a2)));

    STATIC_REQUIRE(!noexcept(b1 = b2));
    STATIC_REQUIRE(!noexcept(b1 = std::move(b2)));
  }

  // Nullopt assignment
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;

    STATIC_REQUIRE(noexcept(a = pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(b = pf::nullopt_holder::value));
  }

  // emplace
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;

    STATIC_REQUIRE(noexcept(a.emplace(42)));
    STATIC_REQUIRE(!noexcept(b.emplace(42)));
  }

  // swap
  {
    pf::optional<NoexceptOps> a1, a2;
    pf::optional<ThrowingOps> b1, b2;

    STATIC_REQUIRE(noexcept(a1.swap(a2)));
    STATIC_REQUIRE(!noexcept(b1.swap(b2)));
  }

  // reset
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;

    STATIC_REQUIRE(noexcept(a.reset()));
    STATIC_REQUIRE(noexcept(b.reset()));
  }

  // has_value, operator bool
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;

    STATIC_REQUIRE(noexcept(a.has_value()));
    STATIC_REQUIRE(noexcept(static_cast<bool>(a)));
    STATIC_REQUIRE(noexcept(b.has_value()));
    STATIC_REQUIRE(noexcept(static_cast<bool>(b)));
  }

  // operator*, operator->
  {
    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};

    STATIC_REQUIRE(noexcept(*a));
    STATIC_REQUIRE(noexcept(a.operator->()));
    STATIC_REQUIRE(noexcept(*b));
    STATIC_REQUIRE(noexcept(b.operator->()));
  }

  // value_or
  {
    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;
    NoexceptOps nv;
    ThrowingOps tv;

    STATIC_REQUIRE(noexcept(a.value_or(nv)));
    STATIC_REQUIRE(!noexcept(b.value_or(tv)));
  }

  // Destructors
  {
    STATIC_REQUIRE(std::is_nothrow_destructible<pf::optional<NoexceptOps>>::value);
    STATIC_REQUIRE(std::is_nothrow_destructible<pf::optional<ThrowingOps>>::value);
  }

  // Monadic operations - and_then
  {
    struct NoexceptCallable {
      pf::optional<int> operator()(NoexceptOps const&) const noexcept { return 42; }
    };
    struct ThrowingCallable {
      pf::optional<int> operator()(ThrowingOps const&) const { return 42; }
    };

    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};
    NoexceptCallable nc;
    ThrowingCallable tc;

    // and_then propagates noexcept from callable
    STATIC_REQUIRE(noexcept(a.and_then(nc)));
    STATIC_REQUIRE(!noexcept(b.and_then(tc)));
  }

  // Monadic operations - transform
  {
    struct NoexceptTransform {
      int operator()(NoexceptOps const&) const noexcept { return 42; }
    };
    struct ThrowingTransform {
      int operator()(ThrowingOps const&) const { return 42; }
    };

    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};
    NoexceptTransform nt;
    ThrowingTransform tt;

    // transform propagates noexcept from callable and result construction
    STATIC_REQUIRE(noexcept(a.transform(nt)));
    STATIC_REQUIRE(!noexcept(b.transform(tt)));
  }

  // Monadic operations - or_else
  {
    struct NoexceptProducer {
      pf::optional<NoexceptOps> operator()() const noexcept { return pf::optional<NoexceptOps>{}; }
    };
    struct ThrowingProducer {
      pf::optional<ThrowingOps> operator()() const { return pf::optional<ThrowingOps>{}; }
    };

    pf::optional<NoexceptOps> a;
    pf::optional<ThrowingOps> b;
    NoexceptProducer np;
    ThrowingProducer tp;

    // or_else propagates noexcept from callable
    STATIC_REQUIRE(noexcept(a.or_else(np)));
    STATIC_REQUIRE(!noexcept(b.or_else(tp)));
  }

  // Iterator operations
  {
    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};

    STATIC_REQUIRE(noexcept(a.begin()));
    STATIC_REQUIRE(noexcept(a.end()));
    STATIC_REQUIRE(noexcept(b.begin()));
    STATIC_REQUIRE(noexcept(b.end()));
  }

  // value() is not noexcept (can throw bad_optional_access)
  {
    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};

    STATIC_REQUIRE(!noexcept(a.value()));
    STATIC_REQUIRE(!noexcept(b.value()));
  }

  // Cross-type construction and assignment
  {
    pf::optional<NoexceptOps> a{pf::in_place_holder::value};
    NoexceptOps nv;

    // Construction from value
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(nv)));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps>(std::move(nv))));

    // Assignment from value
    STATIC_REQUIRE(noexcept(a = nv));
    STATIC_REQUIRE(noexcept(a = std::move(nv)));
  }

  {
    pf::optional<ThrowingOps> b{pf::in_place_holder::value};
    ThrowingOps tv;

    // Construction from value
    STATIC_REQUIRE(!noexcept(pf::optional<ThrowingOps>(tv)));
    STATIC_REQUIRE(!noexcept(pf::optional<ThrowingOps>(std::move(tv))));

    // Assignment from value
    STATIC_REQUIRE(!noexcept(b = tv));
    STATIC_REQUIRE(!noexcept(b = std::move(tv)));
  }
}

TEST_CASE("optional relational operators")
{
  // Comparisons between optional objects
  {
    pf::optional<int> a = 42;
    pf::optional<int> b = 42;
    pf::optional<int> c = 99;
    pf::optional<int> d;

    // operator==
    CHECK((a == b));
    CHECK(!(a == c));
    CHECK(!(a == d));
    CHECK((d == d));

    // operator!=
    CHECK(!(a != b));
    CHECK((a != c));
    CHECK((a != d));
    CHECK(!(d != d));

    // operator< (empty optional is less than any value)
    CHECK(!(a < b));
    CHECK((a < c));
    CHECK(!(a < d));
    CHECK((d < a));
    CHECK(!(d < d));

    // operator<=
    CHECK((a <= b));
    CHECK((a <= c));
    CHECK(!(a <= d));
    CHECK((d <= a));
    CHECK((d <= d));

    // operator>
    CHECK(!(a > b));
    CHECK(!(a > c));
    CHECK((a > d));
    CHECK(!(d > a));
    CHECK(!(d > d));

    // operator>=
    CHECK((a >= b));
    CHECK(!(a >= c));
    CHECK((a >= d));
    CHECK(!(d >= a));
    CHECK((d >= d));
  }

  // Cross-type comparisons
  {
    pf::optional<int> a = 42;
    pf::optional<double> b = 42.0;
    pf::optional<double> c = 99.5;

    CHECK((a == b));
    CHECK(!(a == c));
    CHECK(!(a != b));
    CHECK((a != c));
    CHECK(!(a < b));
    CHECK((a < c));
  }

  // Comparisons with nullopt_t
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    // operator==
    CHECK(!(a == pf::nullopt_holder::value));
    CHECK((pf::nullopt_holder::value == b));
    CHECK((b == pf::nullopt_holder::value));
    CHECK(!(pf::nullopt_holder::value == a));

    // operator!=
    CHECK((a != pf::nullopt_holder::value));
    CHECK(!(pf::nullopt_holder::value != b));
    CHECK(!(b != pf::nullopt_holder::value));
    CHECK((pf::nullopt_holder::value != a));

    // operator<
    CHECK(!(a < pf::nullopt_holder::value));
    CHECK(!(pf::nullopt_holder::value < b));
    CHECK((pf::nullopt_holder::value < a));
    CHECK(!(b < pf::nullopt_holder::value));

    // operator<=
    CHECK(!(a <= pf::nullopt_holder::value));
    CHECK((pf::nullopt_holder::value <= b));
    CHECK((pf::nullopt_holder::value <= a));
    CHECK((b <= pf::nullopt_holder::value));

    // operator>
    CHECK((a > pf::nullopt_holder::value));
    CHECK(!(pf::nullopt_holder::value > b));
    CHECK(!(pf::nullopt_holder::value > a));
    CHECK(!(b > pf::nullopt_holder::value));

    // operator>=
    CHECK((a >= pf::nullopt_holder::value));
    CHECK((pf::nullopt_holder::value >= b));
    CHECK(!(pf::nullopt_holder::value >= a));
    CHECK((b >= pf::nullopt_holder::value));
  }

  // Comparisons with values
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    // operator==
    CHECK((a == 42));
    CHECK((42 == a));
    CHECK(!(a == 99));
    CHECK(!(99 == a));
    CHECK(!(b == 42));
    CHECK(!(42 == b));

    // operator!=
    CHECK(!(a != 42));
    CHECK(!(42 != a));
    CHECK((a != 99));
    CHECK((99 != a));
    CHECK((b != 42));
    CHECK((42 != b));

    // operator< (empty optional is less than any value)
    CHECK(!(a < 42));
    CHECK(!(42 < a));
    CHECK((a < 99));
    CHECK(!(99 < a));
    CHECK((b < 42));
    CHECK(!(42 < b));

    // operator<=
    CHECK((a <= 42));
    CHECK((42 <= a));
    CHECK((a <= 99));
    CHECK(!(99 <= a));
    CHECK((b <= 42));
    CHECK(!(42 <= b));

    // operator>
    CHECK(!(a > 42));
    CHECK(!(42 > a));
    CHECK(!(a > 99));
    CHECK((99 > a));
    CHECK(!(b > 42));
    CHECK((42 > b));

    // operator>=
    CHECK((a >= 42));
    CHECK((42 >= a));
    CHECK(!(a >= 99));
    CHECK((99 >= a));
    CHECK(!(b >= 42));
    CHECK((42 >= b));
  }

  // Cross-type value comparisons
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    CHECK((a == 42.0));
    CHECK((42.0 == a));
    CHECK(!(a == 99.5));
    CHECK(!(99.5 == a));
    CHECK((a < 99.5));
    CHECK(!(99.5 < a));
    CHECK((b < 42.0));
    CHECK(!(42.0 < b));
  }

  // noexcept propagation tests
  {
    struct NoexceptComparable {
      int value;
      constexpr bool operator==(NoexceptComparable const& other) const noexcept { return value == other.value; }
      constexpr bool operator!=(NoexceptComparable const& other) const noexcept { return value != other.value; }
      constexpr bool operator<(NoexceptComparable const& other) const noexcept { return value < other.value; }
      constexpr bool operator<=(NoexceptComparable const& other) const noexcept { return value <= other.value; }
      constexpr bool operator>(NoexceptComparable const& other) const noexcept { return value > other.value; }
      constexpr bool operator>=(NoexceptComparable const& other) const noexcept { return value >= other.value; }
    };

    struct ThrowingComparable {
      int value;
      bool operator==(ThrowingComparable const& other) const { return value == other.value; }
      bool operator!=(ThrowingComparable const& other) const { return value != other.value; }
      bool operator<(ThrowingComparable const& other) const { return value < other.value; }
      bool operator<=(ThrowingComparable const& other) const { return value <= other.value; }
      bool operator>(ThrowingComparable const& other) const { return value > other.value; }
      bool operator>=(ThrowingComparable const& other) const { return value >= other.value; }
    };

    pf::optional<NoexceptComparable> a(pf::in_place_holder::value, NoexceptComparable{42});
    pf::optional<NoexceptComparable> b(pf::in_place_holder::value, NoexceptComparable{42});
    pf::optional<ThrowingComparable> c(pf::in_place_holder::value, ThrowingComparable{42});
    pf::optional<ThrowingComparable> d(pf::in_place_holder::value, ThrowingComparable{42});

    // Verify noexcept propagation for optional vs optional
    STATIC_REQUIRE(noexcept(a == b));
    STATIC_REQUIRE(noexcept(a != b));
    STATIC_REQUIRE(noexcept(a < b));
    STATIC_REQUIRE(noexcept(a <= b));
    STATIC_REQUIRE(noexcept(a > b));
    STATIC_REQUIRE(noexcept(a >= b));

    STATIC_REQUIRE(!noexcept(c == d));
    STATIC_REQUIRE(!noexcept(c != d));
    STATIC_REQUIRE(!noexcept(c < d));
    STATIC_REQUIRE(!noexcept(c <= d));
    STATIC_REQUIRE(!noexcept(c > d));
    STATIC_REQUIRE(!noexcept(c >= d));

    // Verify noexcept for nullopt comparisons (always noexcept)
    STATIC_REQUIRE(noexcept(a == pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(pf::nullopt_holder::value == a));
    STATIC_REQUIRE(noexcept(c == pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(pf::nullopt_holder::value == c));

    // Verify noexcept propagation for optional vs value
    NoexceptComparable nv{42};
    ThrowingComparable tv{42};

    STATIC_REQUIRE(noexcept(a == nv));
    STATIC_REQUIRE(noexcept(nv == a));
    STATIC_REQUIRE(noexcept(a < nv));
    STATIC_REQUIRE(noexcept(nv < a));

    STATIC_REQUIRE(!noexcept(c == tv));
    STATIC_REQUIRE(!noexcept(tv == c));
    STATIC_REQUIRE(!noexcept(c < tv));
    STATIC_REQUIRE(!noexcept(tv < c));
  }
}

TEST_CASE("optional<T&> noexcept propagation")
{
  // Test type with noexcept operations
  struct NoexceptOps {
    NoexceptOps() noexcept = default;
    NoexceptOps(int) noexcept {}
    NoexceptOps(NoexceptOps const&) noexcept = default;
    NoexceptOps(NoexceptOps&&) noexcept = default;
    NoexceptOps& operator=(NoexceptOps const&) noexcept = default;
    NoexceptOps& operator=(NoexceptOps&&) noexcept = default;
    ~NoexceptOps() noexcept = default;
  };

  // Test type with potentially throwing operations
  struct ThrowingOps {
    ThrowingOps() {}
    ThrowingOps(int) {}
    ThrowingOps(ThrowingOps const&) {}
    ThrowingOps(ThrowingOps&&) {}
    ThrowingOps& operator=(ThrowingOps const&) { return *this; }
    ThrowingOps& operator=(ThrowingOps&&) { return *this; }
    ~ThrowingOps() {}
  };

  NoexceptOps nv;
  ThrowingOps tv;

  // Construction
  {
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps&>()));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps&>(pf::nullopt_holder::value)));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps&>(pf::in_place_holder::value, nv)));
    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps&>(nv)));

    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps&>()));
    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps&>(pf::nullopt_holder::value)));
    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps&>(pf::in_place_holder::value, tv)));
    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps&>(tv)));
  }

  // Copy construction (always noexcept for references)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(pf::optional<NoexceptOps&>(a)));
    STATIC_REQUIRE(noexcept(pf::optional<ThrowingOps&>(b)));
  }

  // Copy assignment (always noexcept for references)
  {
    pf::optional<NoexceptOps&> a1{nv}, a2{nv};
    pf::optional<ThrowingOps&> b1{tv}, b2{tv};

    STATIC_REQUIRE(noexcept(a1 = a2));
    STATIC_REQUIRE(noexcept(b1 = b2));
  }

  // Nullopt assignment (always noexcept)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(a = pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(b = pf::nullopt_holder::value));
  }

  // emplace (always noexcept for references)
  {
    pf::optional<NoexceptOps&> a;
    pf::optional<ThrowingOps&> b;

    STATIC_REQUIRE(noexcept(a.emplace(nv)));
    STATIC_REQUIRE(noexcept(b.emplace(tv)));
  }

  // swap (always noexcept for references - just swaps pointers)
  {
    pf::optional<NoexceptOps&> a1{nv}, a2{nv};
    pf::optional<ThrowingOps&> b1{tv}, b2{tv};

    STATIC_REQUIRE(noexcept(a1.swap(a2)));
    STATIC_REQUIRE(noexcept(b1.swap(b2)));
  }

  // reset (always noexcept)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(a.reset()));
    STATIC_REQUIRE(noexcept(b.reset()));
  }

  // has_value, operator bool (always noexcept)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(a.has_value()));
    STATIC_REQUIRE(noexcept(static_cast<bool>(a)));
    STATIC_REQUIRE(noexcept(b.has_value()));
    STATIC_REQUIRE(noexcept(static_cast<bool>(b)));
  }

  // operator*, operator-> (always noexcept)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(*a));
    STATIC_REQUIRE(noexcept(a.operator->()));
    STATIC_REQUIRE(noexcept(*b));
    STATIC_REQUIRE(noexcept(b.operator->()));
  }

  // value_or (propagates noexcept from value type construction)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};
    NoexceptOps nv2;
    ThrowingOps tv2;

    STATIC_REQUIRE(noexcept(a.value_or(nv2)));
    STATIC_REQUIRE(!noexcept(b.value_or(tv2)));
  }

  // value() is not noexcept (can throw bad_optional_access)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(!noexcept(a.value()));
    STATIC_REQUIRE(!noexcept(b.value()));
  }

  // Monadic operations - and_then
  {
    struct NoexceptCallable {
      pf::optional<int> operator()(NoexceptOps&) const noexcept { return 42; }
    };
    struct ThrowingCallable {
      pf::optional<int> operator()(ThrowingOps&) const { return 42; }
    };

    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};
    NoexceptCallable nc;
    ThrowingCallable tc;

    // and_then propagates noexcept from callable
    STATIC_REQUIRE(noexcept(a.and_then(nc)));
    STATIC_REQUIRE(!noexcept(b.and_then(tc)));
  }

  // Monadic operations - transform
  {
    struct NoexceptTransform {
      int operator()(NoexceptOps&) const noexcept { return 42; }
    };
    struct ThrowingTransform {
      int operator()(ThrowingOps&) const { return 42; }
    };

    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};
    NoexceptTransform nt;
    ThrowingTransform tt;

    // transform propagates noexcept from callable
    STATIC_REQUIRE(noexcept(a.transform(nt)));
    STATIC_REQUIRE(!noexcept(b.transform(tt)));
  }

  // Monadic operations - or_else
  {
    struct NoexceptProducer {
      pf::optional<NoexceptOps&> operator()() const noexcept { return pf::optional<NoexceptOps&>{}; }
    };
    struct ThrowingProducer {
      pf::optional<ThrowingOps&> operator()() const { return pf::optional<ThrowingOps&>{}; }
    };

    pf::optional<NoexceptOps&> a;
    pf::optional<ThrowingOps&> b;
    NoexceptProducer np;
    ThrowingProducer tp;

    // or_else propagates noexcept from callable
    STATIC_REQUIRE(noexcept(a.or_else(np)));
    STATIC_REQUIRE(!noexcept(b.or_else(tp)));
  }

  // Iterator operations (always noexcept)
  {
    pf::optional<NoexceptOps&> a{nv};
    pf::optional<ThrowingOps&> b{tv};

    STATIC_REQUIRE(noexcept(a.begin()));
    STATIC_REQUIRE(noexcept(a.end()));
    STATIC_REQUIRE(noexcept(b.begin()));
    STATIC_REQUIRE(noexcept(b.end()));
  }
}

TEST_CASE("optional iterator comparison operators")
{
  // Test iterator comparisons for optional<T>
  {
    pf::optional<int> a = 42;
    pf::optional<int> b;

    auto it1 = a.begin();
    auto it2 = a.end();
    auto it3 = a.begin();

    // Equality
    CHECK((it1 == it3));
    CHECK_FALSE((it1 == it2));

    // Inequality
    CHECK((it1 != it2));
    CHECK_FALSE((it1 != it3));

    // Less than
    CHECK((it1 < it2));
    CHECK_FALSE((it2 < it1));
    CHECK_FALSE((it1 < it3));

    // Less than or equal
    CHECK((it1 <= it2));
    CHECK((it1 <= it3));
    CHECK_FALSE((it2 <= it1));

    // Greater than
    CHECK((it2 > it1));
    CHECK_FALSE((it1 > it2));
    CHECK_FALSE((it1 > it3));

    // Greater than or equal
    CHECK((it2 >= it1));
    CHECK((it1 >= it3));
    CHECK_FALSE((it1 >= it2));

    // Empty optional
    auto empty_begin = b.begin();
    auto empty_end = b.end();
    CHECK((empty_begin == empty_end));
  }

  // Test iterator comparisons for optional<T&>
  {
    int x = 42;
    pf::optional<int&> a{x};
    pf::optional<int&> b;

    auto it1 = a.begin();
    auto it2 = a.end();
    auto it3 = a.begin();

    // Equality
    CHECK((it1 == it3));
    CHECK_FALSE((it1 == it2));

    // Inequality
    CHECK((it1 != it2));
    CHECK_FALSE((it1 != it3));

    // Less than
    CHECK((it1 < it2));
    CHECK_FALSE((it2 < it1));
    CHECK_FALSE((it1 < it3));

    // Less than or equal
    CHECK((it1 <= it2));
    CHECK((it1 <= it3));
    CHECK_FALSE((it2 <= it1));

    // Greater than
    CHECK((it2 > it1));
    CHECK_FALSE((it1 > it2));
    CHECK_FALSE((it1 > it3));

    // Greater than or equal
    CHECK((it2 >= it1));
    CHECK((it1 >= it3));
    CHECK_FALSE((it1 >= it2));

    // Empty optional
    auto empty_begin = b.begin();
    auto empty_end = b.end();
    CHECK((empty_begin == empty_end));
  }

  // Test noexcept specifications
  {
    pf::optional<int> a = 42;
    auto it1 = a.begin();
    auto it2 = a.end();

    STATIC_REQUIRE(noexcept(it1 == it2));
    STATIC_REQUIRE(noexcept(it1 != it2));
    STATIC_REQUIRE(noexcept(it1 < it2));
    STATIC_REQUIRE(noexcept(it1 <= it2));
    STATIC_REQUIRE(noexcept(it1 > it2));
    STATIC_REQUIRE(noexcept(it1 >= it2));
  }
}

TEST_CASE("optional constexpr C++11")
{
  // C++11: constexpr construction and basic operations with literal types
  {
    // Default construction
    constexpr pf::optional<int> empty;
    STATIC_REQUIRE(!empty.has_value());

    // Nullopt construction
    constexpr pf::optional<int> null_opt(pf::nullopt_holder::value);
    STATIC_REQUIRE(!null_opt.has_value());

    // In-place value construction
    constexpr pf::optional<int> opt_value(pf::in_place_holder::value, 42);
    STATIC_REQUIRE(opt_value.has_value());
    STATIC_REQUIRE(*opt_value == 42);

    // Copy construction from empty
    constexpr pf::optional<int> empty_copy(empty);
    STATIC_REQUIRE(!empty_copy.has_value());

    // Copy construction from engaged
    constexpr pf::optional<int> opt_copy(opt_value);
    STATIC_REQUIRE(opt_copy.has_value());
    STATIC_REQUIRE(*opt_copy == 42);

    // Converting construction
    constexpr pf::optional<double> opt_convert(opt_value);
    STATIC_REQUIRE(opt_convert.has_value());
    STATIC_REQUIRE(*opt_convert == 42.0);

    // Dereference operator
    STATIC_REQUIRE(*opt_value == 42);

    // Boolean conversion
    STATIC_REQUIRE(static_cast<bool>(opt_value));
    STATIC_REQUIRE(!static_cast<bool>(empty));
    STATIC_REQUIRE(opt_value.has_value());
    STATIC_REQUIRE(!empty.has_value());
  }

  // C++11: constexpr comparison operators
  {
    constexpr pf::optional<int> opt1(pf::in_place_holder::value, 42);
    constexpr pf::optional<int> opt2(pf::in_place_holder::value, 42);
    constexpr pf::optional<int> opt3(pf::in_place_holder::value, 99);
    constexpr pf::optional<int> empty;

    // optional vs optional
    STATIC_REQUIRE(opt1 == opt2);
    STATIC_REQUIRE(opt1 != opt3);
    STATIC_REQUIRE(opt1 < opt3);
    STATIC_REQUIRE(opt1 <= opt2);
    STATIC_REQUIRE(opt3 > opt1);
    STATIC_REQUIRE(opt1 >= opt2);
    STATIC_REQUIRE(empty != opt1);
    STATIC_REQUIRE(empty == empty);

    // optional vs nullopt
    STATIC_REQUIRE(opt1 != pf::nullopt_holder::value);
    STATIC_REQUIRE(empty == pf::nullopt_holder::value);
    STATIC_REQUIRE(pf::nullopt_holder::value < opt1);
    STATIC_REQUIRE(pf::nullopt_holder::value <= empty);
    STATIC_REQUIRE(opt1 > pf::nullopt_holder::value);

    // optional vs value
    STATIC_REQUIRE(opt1 == 42);
    STATIC_REQUIRE(42 == opt1);
    STATIC_REQUIRE(opt1 != 99);
    STATIC_REQUIRE(opt1 < 99);
    STATIC_REQUIRE(10 < opt1);
    STATIC_REQUIRE(empty < 42);
    STATIC_REQUIRE(42 > empty);
  }
}

TEST_CASE("optional<T&> constexpr C++11")
{
  // Note: Reference optionals have limited constexpr support in C++11
  // Most operations require C++14 or later
  {
    constexpr pf::optional<int&> empty;
    STATIC_REQUIRE(!empty.has_value());

    constexpr pf::optional<int&> null_opt(pf::nullopt_holder::value);
    STATIC_REQUIRE(!null_opt.has_value());
  }
}
