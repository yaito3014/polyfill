#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/extension/toptional.hpp>

namespace pf = yk::polyfill;
namespace ext = pf::extension;

// Custom trait that considers -1 as tombstone
template<class T>
struct minus_one_traits {
  static constexpr bool is_engaged(T const& x) noexcept { return x != -1; }
  static constexpr T tombstone_value() noexcept { return T{-1}; }
};

TEST_CASE("toptional")
{
  STATIC_REQUIRE(sizeof(ext::toptional<int>) == sizeof(int));
  {
    ext::toptional<int> opt;
    CHECK(!opt.has_value());
  }
  {
    ext::toptional<int> opt = 42;
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }
  CHECK_THROWS_AS(ext::toptional<int>{0}, ext::bad_toptional_initialization);

  STATIC_REQUIRE(sizeof(ext::toptional<int*>) == sizeof(int*));
  {
    int x = 42;
    int* ptr = &x;
    ext::toptional<int*> opt = ptr;
    CHECK(opt.has_value());
    CHECK(*opt == ptr);
  }
  CHECK_THROWS_AS(ext::toptional<int*>{nullptr}, ext::bad_toptional_initialization);
}

TEST_CASE("toptional - conversion")
{
  SECTION("copy constructor")
  {
    ext::toptional<double> d = 3.14;
    ext::toptional<int> i = d;
  }

  SECTION("move constructor")
  {
    ext::toptional<double> d = 3.14;
    ext::toptional<int> i = std::move(d);
  }

  SECTION("copy assignment")
  {
    ext::toptional<double> d = 3.14;
    ext::toptional<int> i;
    i = d;
  }

  SECTION("move assignment")
  {
    ext::toptional<double> d = 3.14;
    ext::toptional<int> i;
    i = std::move(d);
  }
}

TEST_CASE("toptional - monadic operations")
{
  SECTION("and_then")
  {
    auto f = [](int x) { return ext::toptional<int>{x * 2}; };
    auto g = [](int) { return ext::toptional<int>{}; };

    ext::toptional<int> opt1 = 21;
    ext::toptional<int> opt2{};

    // Engaged toptional
    auto result1 = opt1.and_then(f);
    CHECK(result1.has_value());
    CHECK(*result1 == 42);

    // Engaged toptional returning nullopt
    auto result2 = opt1.and_then(g);
    CHECK(!result2.has_value());

    // Disengaged toptional
    auto result3 = opt2.and_then(f);
    CHECK(!result3.has_value());
  }

  SECTION("transform")
  {
    auto f = [](int x) { return x * 2; };

    ext::toptional<int> opt1 = 21;
    ext::toptional<int> opt2{};

    // Engaged toptional
    auto result1 = opt1.transform(f);
    CHECK(result1.has_value());
    CHECK(*result1 == 42);

    // Disengaged toptional
    auto result2 = opt2.transform(f);
    CHECK(!result2.has_value());

    // Transform to different type
    auto to_string = [](int x) { return x + 1; };
    auto result3 = opt1.transform(to_string);
    CHECK(result3.has_value());
    CHECK(*result3 == 22);
  }

  SECTION("or_else")
  {
    auto f = []() { return ext::toptional<int>{99}; };

    ext::toptional<int> opt1 = 42;
    ext::toptional<int> opt2{};

    // Engaged toptional
    auto result1 = opt1.or_else(f);
    CHECK(result1.has_value());
    CHECK(*result1 == 42);

    // Disengaged toptional
    auto result2 = opt2.or_else(f);
    CHECK(result2.has_value());
    CHECK(*result2 == 99);
  }

  SECTION("chaining monadic operations")
  {
    auto double_val = [](int x) { return x * 2; };
    auto safe_div = [](int x) {
      if (x == 0) return ext::toptional<int>{};
      return ext::toptional<int>{100 / x};
    };

    ext::toptional<int> opt = 5;
    auto result = opt.transform(double_val).and_then(safe_div);
    CHECK(result.has_value());
    CHECK(*result == 10);  // 5 * 2 = 10, then 100 / 10 = 10

    ext::toptional<int> opt2 = 50;
    auto result2 = opt2.transform(double_val).and_then(safe_div);
    CHECK(result2.has_value());  // 50 * 2 = 100, then 100 / 100 = 1

    ext::toptional<int> opt3{};
    auto result3 = opt3.transform(double_val).and_then(safe_div);
    CHECK(!result3.has_value());
  }
}

TEST_CASE("toptional - iterator support")
{
  SECTION("engaged toptional")
  {
    ext::toptional<int> opt = 42;

    // Range-based for loop
    int count = 0;
    for (int val : opt) {
      CHECK(val == 42);
      count++;
    }
    CHECK(count == 1);

    // Iterator access
    CHECK(opt.begin() != opt.end());
    CHECK(*opt.begin() == 42);
    CHECK(opt.end() - opt.begin() == 1);
  }

  SECTION("disengaged toptional")
  {
    ext::toptional<int> opt{};

    // Range-based for loop (should not execute)
    int count = 0;
    for (int _ : opt) {
      count++;
    }
    CHECK(count == 0);

    // Iterator access
    CHECK(opt.begin() == opt.end());
    CHECK(opt.end() - opt.begin() == 0);
  }

  SECTION("const iterator")
  {
    ext::toptional<int> const opt = 42;

    int count = 0;
    for (int val : opt) {
      CHECK(val == 42);
      count++;
    }
    CHECK(count == 1);
  }
}

TEST_CASE("toptional - assignment operations")
{
  SECTION("assign to engaged toptional")
  {
    ext::toptional<int> opt = 42;
    CHECK(opt.has_value());
    CHECK(*opt == 42);

    opt = 100;
    CHECK(opt.has_value());
    CHECK(*opt == 100);

    CHECK_THROWS_AS(opt = 0, ext::bad_toptional_initialization);
  }

  SECTION("assign to disengaged toptional")
  {
    ext::toptional<int> opt{};
    CHECK(!opt.has_value());

    opt = 42;
    CHECK(opt.has_value());
    CHECK(*opt == 42);
  }

  SECTION("assign from other toptional")
  {
    ext::toptional<int> opt1 = 42;
    ext::toptional<int> opt2 = 100;

    opt1 = opt2;
    CHECK(opt1.has_value());
    CHECK(*opt1 == 100);

    ext::toptional<int> opt3{};
    opt3 = opt2;
    CHECK(opt3.has_value());
    CHECK(*opt3 == 100);
  }
}

TEST_CASE("toptional - emplace")
{
  ext::toptional<int> opt = 42;
  CHECK(*opt == 42);

  opt.emplace(100);
  CHECK(*opt == 100);

  CHECK_THROWS_AS(opt.emplace(0), ext::bad_toptional_initialization);
}

TEST_CASE("toptional - value and value_or")
{
  SECTION("value()")
  {
    ext::toptional<int> opt = 42;
    CHECK(opt.value() == 42);

    ext::toptional<int> opt2{};
    CHECK_THROWS_AS(opt2.value(), yk::polyfill::bad_optional_access);
  }

  SECTION("value_or()")
  {
    ext::toptional<int> opt = 42;
    CHECK(opt.value_or(99) == 42);

    ext::toptional<int> opt2{};
    CHECK(opt2.value_or(99) == 99);
  }
}

TEST_CASE("toptional - reset")
{
  ext::toptional<int> opt = 42;
  CHECK(opt.has_value());

  opt.reset();
  CHECK(!opt.has_value());
}

TEST_CASE("toptional - transform with custom traits")
{
  SECTION("transform with default traits")
  {
    ext::toptional<int> opt = 5;
    auto result = opt.transform([](int x) { return x * 2; });
    CHECK(result.has_value());
    CHECK(*result == 10);

    // Result uses non_zero_traits, so 0 is tombstone
    static_assert(std::is_same<decltype(result), ext::toptional<int, ext::non_zero_traits<int>>>::value, "Default should use non_zero_traits");
  }

  SECTION("transform with custom traits")
  {
    ext::toptional<int> opt = 5;
    auto result = opt.transform<minus_one_traits<int>>([](int x) { return x * 2; });
    CHECK(result.has_value());
    CHECK(*result == 10);

    // Result uses minus_one_traits, so -1 is tombstone
    static_assert(std::is_same<decltype(result), ext::toptional<int, minus_one_traits<int>>>::value, "Should use minus_one_traits");

    // Can construct with 0 (valid value with minus_one_traits)
    ext::toptional<int, minus_one_traits<int>> zero_opt = 0;
    CHECK(zero_opt.has_value());
    CHECK(*zero_opt == 0);

    // Cannot construct with -1 (tombstone with minus_one_traits)
    CHECK_THROWS_AS((ext::toptional<int, minus_one_traits<int>>{-1}), ext::bad_toptional_initialization);
  }

  SECTION("chaining transforms with different traits")
  {
    ext::toptional<int> opt = 5;

    // First transform with default traits (0 is tombstone)
    auto step1 = opt.transform([](int x) { return x + 5; });  // 10
    CHECK(step1.has_value());
    CHECK(*step1 == 10);

    // Second transform with custom traits (-1 is tombstone)
    auto step2 = step1.transform<minus_one_traits<int>>([](int x) { return x - 5; });  // 5
    CHECK(step2.has_value());
    CHECK(*step2 == 5);

    // Third transform back to default traits
    // This should throw because the result (0) is the tombstone value for non_zero_traits
    CHECK_THROWS_AS(step2.transform([](int x) { return x - 5; }), ext::bad_toptional_initialization);
  }
}

// Non-trivial test types
namespace {

struct LifetimeTracker {
  static int constructions;
  static int destructions;
  static int copies;
  static int moves;

  static void reset() { constructions = destructions = copies = moves = 0; }

  int value;

  explicit LifetimeTracker(int v = 1) : value(v) { ++constructions; }
  ~LifetimeTracker() { ++destructions; }

  LifetimeTracker(LifetimeTracker const& other) : value(other.value)
  {
    ++constructions;
    ++copies;
  }

  LifetimeTracker(LifetimeTracker&& other) noexcept : value(other.value)
  {
    ++constructions;
    ++moves;
  }

  LifetimeTracker& operator=(LifetimeTracker const& other)
  {
    value = other.value;
    ++copies;
    return *this;
  }

  LifetimeTracker& operator=(LifetimeTracker&& other) noexcept
  {
    value = other.value;
    ++moves;
    return *this;
  }
};

int LifetimeTracker::constructions = 0;
int LifetimeTracker::destructions = 0;
int LifetimeTracker::copies = 0;
int LifetimeTracker::moves = 0;

// Custom traits for LifetimeTracker (value 0 is tombstone)
struct lifetime_tracker_traits {
  static bool is_engaged(LifetimeTracker const& x) noexcept { return x.value != 0; }
  static LifetimeTracker tombstone_value() noexcept { return LifetimeTracker{0}; }
};

struct NonTrivialWithDestructor {
  static int destructor_calls;
  int value;

  explicit NonTrivialWithDestructor(int v) : value(v) {}

  ~NonTrivialWithDestructor() { ++destructor_calls; }

  NonTrivialWithDestructor(NonTrivialWithDestructor const& other) : value(other.value) {}

  NonTrivialWithDestructor(NonTrivialWithDestructor&& other) noexcept : value(other.value) {}

  NonTrivialWithDestructor& operator=(NonTrivialWithDestructor const& other)
  {
    value = other.value;
    return *this;
  }

  NonTrivialWithDestructor& operator=(NonTrivialWithDestructor&& other) noexcept
  {
    value = other.value;
    return *this;
  }
};

int NonTrivialWithDestructor::destructor_calls = 0;

// Traits for NonTrivialWithDestructor (value -1 is tombstone)
struct non_trivial_traits {
  static bool is_engaged(NonTrivialWithDestructor const& x) noexcept { return x.value != -1; }
  static NonTrivialWithDestructor tombstone_value() noexcept { return NonTrivialWithDestructor{-1}; }
};
}  // namespace

TEST_CASE("toptional - non-trivial types with lifetime tracking")
{
  SECTION("basic construction and destruction")
  {
    LifetimeTracker::reset();

    {
      ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt{pf::in_place_holder::value, 42};
      CHECK(opt.has_value());
      CHECK(opt->value == 42);
      CHECK(LifetimeTracker::constructions >= 1);
    }

    // Destructor should be called when opt goes out of scope
    CHECK(LifetimeTracker::destructions >= 1);
  }

  SECTION("disengaged state doesn't call destructor on tombstone")
  {
    LifetimeTracker::reset();

    {
      ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt;
      CHECK(!opt.has_value());
    }

    // Destructor is still called for the tombstone value in storage
    CHECK(LifetimeTracker::destructions > 0);
  }

  SECTION("copy construction")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt1{pf::in_place_holder::value, 42};
    int copies_before = LifetimeTracker::copies;

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt2 = opt1;

    CHECK(opt2.has_value());
    CHECK(opt2->value == 42);
    CHECK(LifetimeTracker::copies > copies_before);
  }

  SECTION("move construction")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt1{pf::in_place_holder::value, 42};
    int moves_before = LifetimeTracker::moves;

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt2 = std::move(opt1);

    CHECK(opt2.has_value());
    CHECK(opt2->value == 42);
    CHECK(LifetimeTracker::moves > moves_before);
  }

  SECTION("copy assignment")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt1{pf::in_place_holder::value, 42};
    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt2{pf::in_place_holder::value, 99};

    int copies_before = LifetimeTracker::copies;
    opt2 = opt1;

    CHECK(opt2.has_value());
    CHECK(opt2->value == 42);
    CHECK(LifetimeTracker::copies > copies_before);
  }

  SECTION("move assignment")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt1{pf::in_place_holder::value, 42};
    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt2{pf::in_place_holder::value, 99};

    int moves_before = LifetimeTracker::moves;
    opt2 = std::move(opt1);

    CHECK(opt2.has_value());
    CHECK(opt2->value == 42);
    CHECK(LifetimeTracker::moves > moves_before);
  }

  SECTION("emplace")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt{pf::in_place_holder::value, 42};
    CHECK(opt->value == 42);

    int destructions_before = LifetimeTracker::destructions;
    opt.emplace(99);

    CHECK(opt.has_value());
    CHECK(opt->value == 99);
    // Old value should be destroyed
    CHECK(LifetimeTracker::destructions > destructions_before);
  }

  SECTION("reset")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt{pf::in_place_holder::value, 42};
    CHECK(opt.has_value());

    int destructions_before = LifetimeTracker::destructions;
    opt.reset();

    CHECK(!opt.has_value());
    // Old value should be destroyed
    CHECK(LifetimeTracker::destructions > destructions_before);
  }

  SECTION("monadic operations preserve lifetime semantics")
  {
    LifetimeTracker::reset();

    ext::toptional<LifetimeTracker, lifetime_tracker_traits> opt{pf::in_place_holder::value, 5};

    auto result = opt.transform<lifetime_tracker_traits>([](LifetimeTracker const& lt) { return LifetimeTracker{lt.value * 2}; });

    CHECK(result.has_value());
    CHECK(result->value == 10);
    CHECK(LifetimeTracker::constructions > 0);
  }
}

TEST_CASE("toptional - non-trivial type with destructor side effects")
{
  SECTION("destructor called on emplace")
  {
    NonTrivialWithDestructor::destructor_calls = 0;

    ext::toptional<NonTrivialWithDestructor, non_trivial_traits> opt{pf::in_place_holder::value, 42};
    CHECK(opt.has_value());
    CHECK(opt->value == 42);

    int calls_before = NonTrivialWithDestructor::destructor_calls;
    opt.emplace(99);

    CHECK(opt.has_value());
    CHECK(opt->value == 99);
    // Old value's destructor should have been called
    CHECK(NonTrivialWithDestructor::destructor_calls > calls_before);
  }

  SECTION("destructor called on reset")
  {
    NonTrivialWithDestructor::destructor_calls = 0;

    ext::toptional<NonTrivialWithDestructor, non_trivial_traits> opt{pf::in_place_holder::value, 42};
    CHECK(opt.has_value());

    int calls_before = NonTrivialWithDestructor::destructor_calls;
    opt.reset();

    CHECK(!opt.has_value());
    CHECK(NonTrivialWithDestructor::destructor_calls > calls_before);
  }

  SECTION("destructor called when toptional goes out of scope")
  {
    NonTrivialWithDestructor::destructor_calls = 0;

    {
      ext::toptional<NonTrivialWithDestructor, non_trivial_traits> opt{pf::in_place_holder::value, 42};
      CHECK(opt.has_value());
    }

    // Destructor should have been called
    CHECK(NonTrivialWithDestructor::destructor_calls > 0);
  }

  SECTION("move construction preserves value")
  {
    NonTrivialWithDestructor::destructor_calls = 0;

    ext::toptional<NonTrivialWithDestructor, non_trivial_traits> opt1{pf::in_place_holder::value, 42};
    ext::toptional<NonTrivialWithDestructor, non_trivial_traits> opt2 = std::move(opt1);

    CHECK(opt2.has_value());
    CHECK(opt2->value == 42);
  }
}
