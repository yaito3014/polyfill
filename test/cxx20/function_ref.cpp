#include <catch2/catch_test_macros.hpp>

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/type_traits.hpp>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) { return 2 * x; }

struct S {
  int mem(int x) { return 3 * x; }
  int cmem(int x) const { return 4 * x; }
};

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
