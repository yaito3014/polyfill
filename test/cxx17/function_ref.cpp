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
