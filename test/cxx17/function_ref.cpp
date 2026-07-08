#if YK_POLYFILL_CATCH2_MAJOR_VERSION < 3
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

#include <type_traits>

namespace pf = yk::polyfill;

namespace {

int doubles(int x) noexcept { return 2 * x; }

struct S {
  int mem(int x) { return 3 * x; }
  int cmem(int x) const { return 4 * x; }
  int val = 7;
};

int freefn(S&, int x) { return 5 * x; }
int freefn_noexcept(S&, int x) noexcept { return 6 * x; }

// ref-qualified member functions and an unrelated type, to probe the deduction-guide boundary.
struct RefQual {
  int lmem(int x) & { return 3 * x; }
  int rmem(int) && { return 0; }
};
struct Unrelated {};

// Whether the two-argument constant_wrapper deduction guide [func.wrap.ref.deduct] applies,
// i.e. whether cw_deduced_signature<F, T> yields a signature.
template<class F, class T, class = void>
struct guide_applies : std::false_type {};
template<class F, class T>
struct guide_applies<F, T, std::void_t<typename pf::detail::cw_deduced_signature<F, T>::type>> : std::true_type {};

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

TEST_CASE("function_ref constant_wrapper")
{
  // constant free function pointer, no bound object (doubles is noexcept here)
  {
    pf::function_ref<int(int) noexcept> const ref = pf::cw<&doubles>;
    CHECK(ref(21) == 42);
  }

  // CTAD guide 1: function_ref(constant_wrapper<c, F0>) -> function_ref<remove_pointer_t<F0>>
  {
    pf::function_ref ref = pf::cw<&doubles>;
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int(int) noexcept>>);
    CHECK(ref(21) == 42);
  }

  // constant member function pointer bound to an lvalue object / to a pointer
  {
    S s;
    pf::function_ref<int(int)> const ref{pf::cw<&S::mem>, s};
    CHECK(ref(14) == 42);
  }
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

TEST_CASE("function_ref constant_wrapper two-argument CTAD boundary")
{
  // [func.wrap.ref.deduct]: the member-function form is R(G::*)(A...) cv opt& noexcept(E).
  // An lvalue-ref-qualified member function is of that form; an rvalue-ref-qualified one is not.
  STATIC_REQUIRE(guide_applies<decltype(&RefQual::lmem), RefQual>::value);
  STATIC_REQUIRE_FALSE(guide_applies<decltype(&RefQual::rmem), RefQual>::value);

  // The member-object form deduces only when F is invocable with T&, so binding an unrelated
  // object drops the guide (rather than hard-erroring).
  STATIC_REQUIRE(guide_applies<decltype(&S::val), S>::value);
  STATIC_REQUIRE_FALSE(guide_applies<decltype(&S::val), Unrelated>::value);

  // the lvalue-ref-qualified member function still deduces and invokes through CTAD.
  {
    RefQual rq;
    pf::function_ref ref{pf::cw<&RefQual::lmem>, rq};
    STATIC_REQUIRE(std::is_same_v<decltype(ref), pf::function_ref<int(int)>>);
    CHECK(ref(14) == 42);
  }
}
