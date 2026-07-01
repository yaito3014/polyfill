#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) noexcept { return 2 * x; }

struct NoexceptCallable {
  int operator()(int x) const noexcept { return 2 * x; }
};

struct DifferentForConstnessNoexcept {
  int operator()(int x) const noexcept { return 2 * x; }
  int operator()(int x) noexcept { return 3 * x; }
};

struct ConstexprCallable {
  constexpr int operator()(int x) const { return 2 * x; }
};

}  // namespace

TEST_CASE("function_ref noexcept")
{
  // noexcept function
  {
    pf::function_ref<int(int) noexcept> const ref = doubles;
    CHECK(ref(21) == 42);
  }

  // noexcept callable
  {
    NoexceptCallable const func;
    pf::function_ref<int(int) noexcept> const ref = func;
    CHECK(ref(21) == 42);
  }
  {
    NoexceptCallable const func;
    pf::function_ref<int(int) const noexcept> const ref = func;
    CHECK(ref(21) == 42);
  }

  // noexcept const-propagation
  {
    DifferentForConstnessNoexcept func;
    pf::function_ref<int(int) noexcept> const ref = func;
    CHECK(ref(21) == 63);
  }
  {
    DifferentForConstnessNoexcept func;
    pf::function_ref<int(int) const noexcept> const ref = func;
    CHECK(ref(21) == 42);
  }
  {
    DifferentForConstnessNoexcept const func;
    pf::function_ref<int(int) const noexcept> const ref = func;
    CHECK(ref(21) == 42);
  }

  // noexcept function pointer
  {
    pf::function_ref<int(int) noexcept> const ref = &doubles;
    CHECK(ref(21) == 42);
  }
}

TEST_CASE("function_ref cross-specialization conversion (noexcept)")
{
  // noexcept -> potentially-throwing: is-convertible-from-specialization is true, so
  // the target adopts the source's bound entity even from a temporary.
  {
    pf::function_ref<int(int)> const ref = pf::function_ref<int(int) noexcept>(doubles);
    CHECK(ref(21) == 42);
  }
  // const noexcept -> non-const potentially-throwing: widest source to narrowest target.
  {
    NoexceptCallable func;
    pf::function_ref<int(int)> const ref = pf::function_ref<int(int) const noexcept>(func);
    CHECK(ref(21) == 42);
  }
  {
    NoexceptCallable func;
    pf::function_ref<int(int) const noexcept> const src = func;
    pf::function_ref<int(int) noexcept> const ref = src;
    CHECK(ref(21) == 42);
  }

  // potentially-throwing -> noexcept is not convertible-from-specialization, but adding
  // const (noexcept -> const noexcept) is still a valid wrapping conversion from an lvalue.
  {
    pf::function_ref<int(int) noexcept> const src = doubles;
    pf::function_ref<int(int) const noexcept> const ref = src;
    CHECK(ref(21) == 42);
  }

  // adopting another specialization is usable in a constant expression.
  {
    static constexpr ConstexprCallable func{};
    constexpr pf::function_ref<int(int) const> src = func;
    constexpr pf::function_ref<int(int)> ref = src;
    CHECK(ref(21) == 42);
  }
}

TEST_CASE("function_ref CTAD")
{
  // deduction from function pointer
  {
    pf::function_ref ref = doubles;
    CHECK(ref(21) == 42);
  }
  {
    pf::function_ref ref = &doubles;
    CHECK(ref(21) == 42);
  }
}
