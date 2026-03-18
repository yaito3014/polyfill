#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) { return 2 * x; }

struct DifferentForConstness {
  int operator()(int x) const { return 2 * x; }
  int operator()(int x) { return 3 * x; }
};

struct OnlyMutable {
  int operator()(int x) { return 2 * x; }
};

}  // namespace

TEST_CASE("function_ref")
{
  // function
  {
    pf::function_ref<int(int)> const ref = doubles;
    CHECK(ref(21) == 42);
  }

  // lambda without capture
  {
    auto const func = [](int x) -> int { return 2 * x; };
    pf::function_ref<int(int)> const ref = func;
    CHECK(ref(21) == 42);
  }

  // lambda with capture
  {
    int const multiple = 2;
    auto const func = [multiple](int x) -> int { return multiple * x; };
    pf::function_ref<int(int)> const ref = func;
    CHECK(ref(21) == 42);
  }

  // const-propagation
  {
    DifferentForConstness const func;
    pf::function_ref<int(int)> const ref = func;  // looks weird, but the standard allows
    CHECK(ref(21) == 42);
  }
  {
    DifferentForConstness const func;
    pf::function_ref<int(int) const> const ref = func;
    CHECK(ref(21) == 42);
  }
  {
    DifferentForConstness func;
    pf::function_ref<int(int)> const ref = func;
    CHECK(ref(21) == 63);
  }
  {
    DifferentForConstness func;
    pf::function_ref<int(int) const> const ref = func;
    CHECK(ref(21) == 42);
  }

  {
    OnlyMutable func;
    pf::function_ref<int(int)> const ref = func;
    CHECK(ref(21) == 42);
  }
  // {
  //   OnlyMutable func;
  //   pf::function_ref<int(int) const> const ref = func;
  //   CHECK(ref(21) == 42);
  // }
  // {
  //   OnlyMutable const func;
  //   pf::function_ref<int(int)> const ref = func;
  //   CHECK(ref(21) == 42);
  // }
  // {
  //   OnlyMutable const func;
  //   pf::function_ref<int(int) const> const ref = func;
  //   CHECK(ref(21) == 42);
  // }
}
