#ifndef YK_ZZ_POLYFILL_EXTENSION_INVOCABLE_TRAITS_HPP
#define YK_ZZ_POLYFILL_EXTENSION_INVOCABLE_TRAITS_HPP

#include <cstddef>

namespace yk {

namespace polyfill {

namespace extension {

namespace detail {

// Rebuilds a plain function type from its parts (cv/ref-qualifiers dropped), keeping
// the C-style ellipsis and, where it can be part of the type (C++17), the noexcept.
template<class R, bool IsNoexcept, bool IsVariadic, class... Args>
struct invocable_function_type;
template<class R, class... Args>
struct invocable_function_type<R, false, false, Args...> {
  using type = R(Args...);
};
template<class R, class... Args>
struct invocable_function_type<R, false, true, Args...> {
  using type = R(Args..., ...);
};
#if __cpp_noexcept_function_type >= 201510L
template<class R, class... Args>
struct invocable_function_type<R, true, false, Args...> {
  using type = R(Args...) noexcept;
};
template<class R, class... Args>
struct invocable_function_type<R, true, true, Args...> {
  using type = R(Args..., ...) noexcept;
};
#endif

template<class R, bool IsConst, bool IsVolatile, bool IsLvalueRef, bool IsRvalueRef, bool IsNoexcept, bool IsVariadic, class... Args>
struct invocable_traits_function {
  using return_type = R;

  static constexpr std::size_t arity = sizeof...(Args);
  static constexpr bool is_const = IsConst;
  static constexpr bool is_volatile = IsVolatile;
  static constexpr bool is_lvalue_reference = IsLvalueRef;
  static constexpr bool is_rvalue_reference = IsRvalueRef;
  static constexpr bool is_noexcept = IsNoexcept;
  static constexpr bool is_variadic = IsVariadic;

  template<template<class...> class Tmpl>
  using apply_args = Tmpl<Args...>;

  using function_type = typename invocable_function_type<R, IsNoexcept, IsVariadic, Args...>::type;
};

template<class R, class C, bool IsConst, bool IsVolatile, bool IsLvalueRef, bool IsRvalueRef, bool IsNoexcept, bool IsVariadic, class... Args>
struct invocable_traits_memfn : invocable_traits_function<R, IsConst, IsVolatile, IsLvalueRef, IsRvalueRef, IsNoexcept, IsVariadic, Args...> {
  using class_type = C;
};

}  // namespace detail

// Decomposes a callable type F -- a function pointer, member function pointer, or
// member object pointer -- into its constituent parts. Undefined for other types.
template<class F>
struct invocable_traits;

template<class R, class... Args>
struct invocable_traits<R (*)(Args...)> : detail::invocable_traits_function<R, false, false, false, false, false, false, Args...> {};
template<class R, class... Args>
struct invocable_traits<R (*)(Args..., ...)> : detail::invocable_traits_function<R, false, false, false, false, false, true, Args...> {};

// Emits the plain and the C-variadic member function specialization for one qualifier set.
#define YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(qualifiers, is_const, is_vol, is_lref, is_rref, ne, is_ne)                                             \
  template<class R, class C, class... Args>                                                                                                             \
  struct invocable_traits<R (C::*)(Args...) qualifiers ne> : detail::invocable_traits_memfn<R, C, is_const, is_vol, is_lref, is_rref, is_ne, false, Args...> {}; \
  template<class R, class C, class... Args>                                                                                                             \
  struct invocable_traits<R (C::*)(Args..., ...) qualifiers ne> : detail::invocable_traits_memfn<R, C, is_const, is_vol, is_lref, is_rref, is_ne, true, Args...> {};

YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(, false, false, false, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const, true, false, false, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile, false, true, false, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile, true, true, false, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(&, false, false, true, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const&, true, false, true, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile&, false, true, true, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile&, true, true, true, false, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(&&, false, false, false, true, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const&&, true, false, false, true, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile&&, false, true, false, true, , false)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile&&, true, true, false, true, , false)

#if __cpp_noexcept_function_type >= 201510L
template<class R, class... Args>
struct invocable_traits<R (*)(Args...) noexcept> : detail::invocable_traits_function<R, false, false, false, false, true, false, Args...> {};
template<class R, class... Args>
struct invocable_traits<R (*)(Args..., ...) noexcept> : detail::invocable_traits_function<R, false, false, false, false, true, true, Args...> {};

YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(, false, false, false, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const, true, false, false, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile, false, true, false, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile, true, true, false, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(&, false, false, true, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const&, true, false, true, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile&, false, true, true, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile&, true, true, true, false, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(&&, false, false, false, true, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const&&, true, false, false, true, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(volatile&&, false, true, false, true, noexcept, true)
YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN(const volatile&&, true, true, false, true, noexcept, true)
#endif

#undef YK_POLYFILL_INVOCABLE_TRAITS_DEFINE_MEMFN

template<class M, class C>
struct invocable_traits<M C::*> {
  using class_type = C;
  using member_type = M;
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_EXTENSION_INVOCABLE_TRAITS_HPP
