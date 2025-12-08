#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/extension/toptional.hpp>

#include <compare>

namespace ext = yk::polyfill::extension;

TEST_CASE("toptional three-way comparison")
{
  // Three-way comparison between toptional objects
  {
    ext::toptional<int> a = 42;
    ext::toptional<int> b = 42;
    ext::toptional<int> c = 99;
    ext::toptional<int> d;

    CHECK(((a <=> b) == std::strong_ordering::equal));
    CHECK(((a <=> c) == std::strong_ordering::less));
    CHECK(((c <=> a) == std::strong_ordering::greater));
    CHECK(((d <=> d) == std::strong_ordering::equal));
    CHECK(((d <=> a) == std::strong_ordering::less));
    CHECK(((a <=> d) == std::strong_ordering::greater));
  }

  // Three-way comparison with nullopt_t
  {
    ext::toptional<int> a = 42;
    ext::toptional<int> b;

    CHECK(((a <=> yk::polyfill::nullopt_holder::value) == std::strong_ordering::greater));
    CHECK(((b <=> yk::polyfill::nullopt_holder::value) == std::strong_ordering::equal));
    CHECK(((yk::polyfill::nullopt_holder::value <=> a) == std::strong_ordering::less));
    CHECK(((yk::polyfill::nullopt_holder::value <=> b) == std::strong_ordering::equal));
  }

  // Three-way comparison with values
  {
    ext::toptional<int> a = 42;
    ext::toptional<int> b;

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

    ext::toptional<NoexceptComparable> a = NoexceptComparable{42};
    ext::toptional<NoexceptComparable> b = NoexceptComparable{42};
    ext::toptional<ThrowingComparable> c = ThrowingComparable{42};
    ext::toptional<ThrowingComparable> d = ThrowingComparable{42};

    // Verify noexcept propagation for toptional vs toptional
    STATIC_REQUIRE(noexcept(a <=> b));
    STATIC_REQUIRE(!noexcept(c <=> d));

    // Verify noexcept for nullopt comparisons (always noexcept)
    STATIC_REQUIRE(noexcept(a <=> yk::polyfill::nullopt_holder::value));
    STATIC_REQUIRE(noexcept(c <=> yk::polyfill::nullopt_holder::value));

    // Verify noexcept propagation for toptional vs value
    NoexceptComparable nv{42};
    ThrowingComparable tv{42};

    STATIC_REQUIRE(noexcept(a <=> nv));
    STATIC_REQUIRE(!noexcept(c <=> tv));
  }
}

TEST_CASE("toptional three-way comparison with different traits")
{
  struct bool_traits {
    static bool is_engaged(bool x) noexcept { return x; }
    static bool tombstone_value() noexcept { return false; }
  };

  ext::toptional<bool, bool_traits> a = true;
  ext::toptional<bool, bool_traits> b = true;
  ext::toptional<bool, bool_traits> c;

  CHECK(((a <=> b) == std::strong_ordering::equal));
  CHECK(((c <=> a) == std::strong_ordering::less));
  CHECK(((a <=> c) == std::strong_ordering::greater));
  CHECK(((c <=> c) == std::strong_ordering::equal));
}

TEST_CASE("toptional constexpr C++20")
{
  // C++20: constexpr destructors allow more complex constexpr scenarios
  {
    // Test with non-trivially destructible types
    struct NonTrivial {
      int value;
      constexpr NonTrivial(int v) : value(v) {}
      constexpr ~NonTrivial() {}
    };

    struct non_trivial_traits {
      static constexpr bool is_engaged(NonTrivial const& x) noexcept { return x.value != -1; }
      static constexpr NonTrivial tombstone_value() noexcept { return NonTrivial{-1}; }
    };

    // Construction and destruction
    constexpr auto test_nontrivial_construct = []() {
      ext::toptional<NonTrivial, non_trivial_traits> opt(42);
      return opt.has_value() && opt->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_construct());

    // Assignment with non-trivial type
    constexpr auto test_nontrivial_assign = []() {
      ext::toptional<NonTrivial, non_trivial_traits> opt1(42);
      ext::toptional<NonTrivial, non_trivial_traits> opt2;
      opt2 = opt1;
      return opt2.has_value() && opt2->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_assign());

    // Reset with non-trivial type
    constexpr auto test_nontrivial_reset = []() {
      ext::toptional<NonTrivial, non_trivial_traits> opt(42);
      opt.reset();
      return !opt.has_value();
    };
    STATIC_REQUIRE(test_nontrivial_reset());

    // Emplace with non-trivial type
    constexpr auto test_nontrivial_emplace = []() {
      ext::toptional<NonTrivial, non_trivial_traits> opt(10);
      opt.emplace(42);
      return opt.has_value() && opt->value == 42;
    };
    STATIC_REQUIRE(test_nontrivial_emplace());
  }

  // C++20: Three-way comparison in constexpr context
  {
    constexpr auto test_spaceship = []() {
      ext::toptional<int> a = 42;
      ext::toptional<int> b = 42;
      ext::toptional<int> c = 99;
      ext::toptional<int> empty;

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
      ext::toptional<int> a = 42;
      ext::toptional<int> empty;

      return (a <=> yk::polyfill::nullopt_holder::value) == std::strong_ordering::greater &&
             (empty <=> yk::polyfill::nullopt_holder::value) == std::strong_ordering::equal &&
             (yk::polyfill::nullopt_holder::value <=> a) == std::strong_ordering::less &&
             (yk::polyfill::nullopt_holder::value <=> empty) == std::strong_ordering::equal;
    };
    STATIC_REQUIRE(test_spaceship_nullopt());

    // Three-way comparison with values
    constexpr auto test_spaceship_value = []() {
      ext::toptional<int> a = 42;
      ext::toptional<int> empty;

      return (a <=> 42) == std::strong_ordering::equal &&
             (a <=> 99) == std::strong_ordering::less &&
             (a <=> 10) == std::strong_ordering::greater &&
             (empty <=> 42) == std::strong_ordering::less;
    };
    STATIC_REQUIRE(test_spaceship_value());
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

      struct data_traits {
        static constexpr bool is_engaged(Data const& d) noexcept { return d.x != -1 && d.y != -1; }
        static constexpr Data tombstone_value() noexcept { return Data{-1, -1}; }
      };

      ext::toptional<Data, data_traits> opt1(10, 20);
      ext::toptional<Data, data_traits> opt2;

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
