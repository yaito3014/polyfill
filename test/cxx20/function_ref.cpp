#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) { return 2 * x; }

struct S {
  int mem(int x) { return 3 * x; }
  int cmem(int x) const { return 4 * x; }
  int val = 7;
};

int freefn(S&, int x) { return 5 * x; }
int freefn_noexcept(S&, int x) noexcept { return 6 * x; }

}  // namespace

TEST_CASE("function_ref constant_wrapper")
{
  // constant free function pointer (no bound object)
  {
    pf::function_ref<int(int)> const ref = pf::cw<&doubles>;
    CHECK(ref(21) == 42);
  }

  // CTAD: function_ref(constant_wrapper<c, F0>) -> function_ref<remove_pointer_t<F0>>
  {
    pf::function_ref ref = pf::cw<&doubles>;
    CHECK(ref(21) == 42);
  }

  // constant member function pointer bound to an lvalue object
  {
    S s;
    pf::function_ref<int(int)> const ref{pf::cw<&S::mem>, s};
    CHECK(ref(14) == 42);
  }

  // constant member function pointer bound to a pointer
  {
    S s;
    pf::function_ref<int(int)> const ref{pf::cw<&S::mem>, &s};
    CHECK(ref(14) == 42);
  }

  // const-qualified target through a const function_ref
  {
    S s;
    pf::function_ref<int(int) const> const ref{pf::cw<&S::cmem>, s};
    CHECK(ref(10) == 40);
  }
  {
    S const s;
    pf::function_ref<int(int) const> const ref{pf::cw<&S::cmem>, &s};
    CHECK(ref(10) == 40);
  }
}

TEST_CASE("function_ref constant_wrapper two-argument CTAD")
{
  // member function pointer -> function_ref<int(int)>
  {
    S s;
    pf::function_ref ref{pf::cw<&S::mem>, s};
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int(int)>>);
    CHECK(ref(14) == 42);
  }
  // free function pointer: leading (bound) parameter is dropped
  {
    S s;
    pf::function_ref ref{pf::cw<&freefn>, s};
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int(int)>>);
    CHECK(ref(8) == 40);
  }
  {
    S s;
    pf::function_ref ref{pf::cw<&freefn_noexcept>, s};
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int(int) noexcept>>);
    CHECK(ref(7) == 42);
  }
  // member object pointer -> function_ref<int&() noexcept>
  {
    S s;
    pf::function_ref ref{pf::cw<&S::val>, s};
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int&() noexcept>>);
    CHECK(ref() == 7);
  }
}
