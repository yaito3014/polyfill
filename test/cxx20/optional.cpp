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

  // noexcept propagation tests for three-way comparison
  {
    struct NoexceptComparable {
      int value;
      constexpr auto operator<=>(NoexceptComparable const& other) const noexcept { return value <=> other.value; }
      constexpr bool operator==(NoexceptComparable const& other) const noexcept = default;
    };

    struct ThrowingComparable {
      int value;
      auto operator<=>(ThrowingComparable const& other) const { return value <=> other.value; }
      bool operator==(ThrowingComparable const& other) const = default;
    };

    pf::optional<NoexceptComparable> a{pf::in_place, NoexceptComparable{42}};
    pf::optional<NoexceptComparable> b{pf::in_place, NoexceptComparable{42}};
    pf::optional<ThrowingComparable> c{pf::in_place, ThrowingComparable{42}};
    pf::optional<ThrowingComparable> d{pf::in_place, ThrowingComparable{42}};

    // Verify noexcept propagation for optional vs optional
    STATIC_REQUIRE(noexcept(a <=> b));
    STATIC_REQUIRE(!noexcept(c <=> d));

    // Verify noexcept for nullopt comparisons (always noexcept)
    STATIC_REQUIRE(noexcept(a <=> pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(c <=> pf::nullopt_holder::value));

    // Verify noexcept propagation for optional vs value
    NoexceptComparable nv{42};
    ThrowingComparable tv{42};

    STATIC_REQUIRE(noexcept(a <=> nv));
    STATIC_REQUIRE(!noexcept(c <=> tv));
  }
}

TEST_CASE("optional iterator three-way comparison")
{
  pf::optional<int> a = 42;
  pf::optional<int> b;

  auto it1 = a.begin();
  auto it2 = a.end();
  auto it3 = a.begin();

  // Three-way comparison
  CHECK(((it1 <=> it3) == std::strong_ordering::equal));
  CHECK(((it1 <=> it2) == std::strong_ordering::less));
  CHECK(((it2 <=> it1) == std::strong_ordering::greater));

  // Empty optional iterators
  auto empty_begin = b.begin();
  auto empty_end = b.end();
  CHECK(((empty_begin <=> empty_end) == std::strong_ordering::equal));

  // noexcept specification
  STATIC_REQUIRE(noexcept(it1 <=> it2));
}

TEST_CASE("optional<T&> three-way comparison")
{
  int x = 42;
  int y = 42;
  int z = 99;

  // Three-way comparison between optional<T&> objects
  {
    pf::optional<int&> a{x};
    pf::optional<int&> b{y};
    pf::optional<int&> c{z};
    pf::optional<int&> d;

    CHECK(((a <=> b) == std::strong_ordering::equal));
    CHECK(((a <=> c) == std::strong_ordering::less));
    CHECK(((c <=> a) == std::strong_ordering::greater));
    CHECK(((d <=> d) == std::strong_ordering::equal));
    CHECK(((d <=> a) == std::strong_ordering::less));
    CHECK(((a <=> d) == std::strong_ordering::greater));
  }

  // Three-way comparison with nullopt_t
  {
    pf::optional<int&> a{x};
    pf::optional<int&> b;

    CHECK(((a <=> pf::nullopt_holder::value) == std::strong_ordering::greater));
    CHECK(((b <=> pf::nullopt_holder::value) == std::strong_ordering::equal));
    CHECK(((pf::nullopt_holder::value <=> a) == std::strong_ordering::less));
    CHECK(((pf::nullopt_holder::value <=> b) == std::strong_ordering::equal));
  }

  // Three-way comparison with values
  {
    pf::optional<int&> a{x};
    pf::optional<int&> b;

    CHECK(((a <=> 42) == std::strong_ordering::equal));
    CHECK(((a <=> 99) == std::strong_ordering::less));
    CHECK(((a <=> 10) == std::strong_ordering::greater));
    CHECK(((b <=> 42) == std::strong_ordering::less));
  }

  // noexcept propagation tests for three-way comparison
  {
    struct NoexceptComparable {
      int value;
      constexpr auto operator<=>(NoexceptComparable const& other) const noexcept { return value <=> other.value; }
      constexpr bool operator==(NoexceptComparable const& other) const noexcept = default;
    };

    struct ThrowingComparable {
      int value;
      auto operator<=>(ThrowingComparable const& other) const { return value <=> other.value; }
      bool operator==(ThrowingComparable const& other) const = default;
    };

    NoexceptComparable nv1{42};
    NoexceptComparable nv2{42};
    ThrowingComparable tv1{42};
    ThrowingComparable tv2{42};

    pf::optional<NoexceptComparable&> a{nv1};
    pf::optional<NoexceptComparable&> b{nv2};
    pf::optional<ThrowingComparable&> c{tv1};
    pf::optional<ThrowingComparable&> d{tv2};

    // Verify noexcept propagation for optional vs optional
    STATIC_REQUIRE(noexcept(a <=> b));
    STATIC_REQUIRE(!noexcept(c <=> d));

    // Verify noexcept for nullopt comparisons (always noexcept)
    STATIC_REQUIRE(noexcept(a <=> pf::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(c <=> pf::nullopt_holder::value));

    // Verify noexcept propagation for optional vs value
    STATIC_REQUIRE(noexcept(a <=> nv1));
    STATIC_REQUIRE(!noexcept(c <=> tv1));
  }
}

TEST_CASE("optional constexpr C++20")
{
  // C++20: constexpr destructors allow more complex constexpr scenarios
  {
    // Test with non-trivially destructible types
    struct NonTrivial {
      int value;
      constexpr NonTrivial(int v) : value(v) {}
      constexpr ~NonTrivial() {}
    };

    // Construction and destruction
    constexpr auto test_nontrivial_construct = []() {
      pf::optional<NonTrivial> opt(pf::in_place_holder::value, 42);
      return opt.has_value() && opt->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_construct());

    // Assignment with non-trivial type
    constexpr auto test_nontrivial_assign = []() {
      pf::optional<NonTrivial> opt1(pf::in_place_holder::value, 42);
      pf::optional<NonTrivial> opt2;
      opt2 = opt1;
      return opt2.has_value() && opt2->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_assign());

    // Reset with non-trivial type
    constexpr auto test_nontrivial_reset = []() {
      pf::optional<NonTrivial> opt(pf::in_place_holder::value, 42);
      opt.reset();
      return !opt.has_value();
    };
    STATIC_REQUIRE(test_nontrivial_reset());

    // Emplace with non-trivial type
    constexpr auto test_nontrivial_emplace = []() {
      pf::optional<NonTrivial> opt(pf::in_place_holder::value, 10);
      opt.emplace(42);
      return opt.has_value() && opt->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_emplace());
  }

  // C++20: Three-way comparison in constexpr context
  {
    constexpr auto test_spaceship = []() {
      pf::optional<int> a(pf::in_place_holder::value, 42);
      pf::optional<int> b(pf::in_place_holder::value, 42);
      pf::optional<int> c(pf::in_place_holder::value, 99);
      pf::optional<int> empty;

      return (a <=> b) == std::strong_ordering::equal &&
             (a <=> c) == std::strong_ordering::less &&
             (c <=> a) == std::strong_ordering::greater &&
             (empty <=> a) == std::strong_ordering::less &&
             (a <=> empty) == std::strong_ordering::greater &&
             (empty <=> empty) == std::strong_ordering::equal;
    };
    STATIC_REQUIRE(test_spaceship());

    // Three-way comparison with nullopt
    constexpr auto test_spaceship_nullopt = []() {
      pf::optional<int> a(pf::in_place_holder::value, 42);
      pf::optional<int> empty;

      return (a <=> pf::nullopt_holder::value) == std::strong_ordering::greater &&
             (empty <=> pf::nullopt_holder::value) == std::strong_ordering::equal &&
             (pf::nullopt_holder::value <=> a) == std::strong_ordering::less &&
             (pf::nullopt_holder::value <=> empty) == std::strong_ordering::equal;
    };
    STATIC_REQUIRE(test_spaceship_nullopt());

    // Three-way comparison with values
    constexpr auto test_spaceship_value = []() {
      pf::optional<int> a(pf::in_place_holder::value, 42);
      pf::optional<int> empty;

      return (a <=> 42) == std::strong_ordering::equal &&
             (a <=> 99) == std::strong_ordering::less &&
             (a <=> 10) == std::strong_ordering::greater &&
             (empty <=> 42) == std::strong_ordering::less;
    };
    STATIC_REQUIRE(test_spaceship_value());
  }

  // C++20: Iterator three-way comparison
  {
    constexpr auto test_iterator_spaceship = []() {
      pf::optional<int> opt(pf::in_place_holder::value, 42);
      auto it1 = opt.begin();
      auto it2 = opt.end();
      auto it3 = opt.begin();

      return (it1 <=> it3) == std::strong_ordering::equal &&
             (it1 <=> it2) == std::strong_ordering::less &&
             (it2 <=> it1) == std::strong_ordering::greater;
    };
    STATIC_REQUIRE(test_iterator_spaceship());
  }

  // C++20: Complex constexpr scenarios combining multiple operations
  {
    constexpr auto test_complex = []() {
      struct Data {
        int x;
        int y;
        constexpr Data(int a, int b) : x(a), y(b) {}
        constexpr ~Data() {}
      };

      pf::optional<Data> opt1(pf::in_place_holder::value, 10, 20);
      pf::optional<Data> opt2;

      // Copy assign
      opt2 = opt1;

      // Modify through emplace
      opt1.emplace(30, 40);

      // Swap
      opt1.swap(opt2);

      // Check results
      return opt1.has_value() && opt1->x == 10 && opt1->y == 20 &&
             opt2.has_value() && opt2->x == 30 && opt2->y == 40;
    };
    STATIC_REQUIRE(test_complex());
  }
}

TEST_CASE("optional<T&> constexpr C++20")
{
  // C++20: Reference optional with three-way comparison
  {
    constexpr auto test_ref_spaceship = []() {
      int x = 42;
      int y = 42;
      int z = 99;

      pf::optional<int&> a{x};
      pf::optional<int&> b{y};
      pf::optional<int&> c{z};
      pf::optional<int&> empty;

      return (a <=> b) == std::strong_ordering::equal &&
             (a <=> c) == std::strong_ordering::less &&
             (c <=> a) == std::strong_ordering::greater &&
             (empty <=> a) == std::strong_ordering::less;
    };
    STATIC_REQUIRE(test_ref_spaceship());
  }
}
