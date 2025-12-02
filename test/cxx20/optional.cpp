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
