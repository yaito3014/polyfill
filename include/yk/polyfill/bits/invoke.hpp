#ifndef YK_ZZ_POLYFILL_BITS_INVOKE_HPP
#define YK_ZZ_POLYFILL_BITS_INVOKE_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>

#include <functional>
#include <type_traits>

namespace yk {

namespace polyfill {

namespace detail {

template<class T>
struct is_reference_wrapper : false_type {};

template<class T>
struct is_reference_wrapper<std::reference_wrapper<T>> : true_type {};

// normal case
template<class F, class... Args, typename std::enable_if<!std::is_member_pointer<F>::value, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(F&& f, Args&&... args) noexcept(noexcept(static_cast<F&&>(f)(static_cast<Args&&>(args)...)))
    -> decltype(static_cast<F&&>(f)(static_cast<Args&&>(args)...))
{
  return static_cast<F&&>(f)(static_cast<Args&&>(args)...);
}

enum class invoke_kind {
  reference_to_object,
  reference_wrapper,
  other,
};

template<class C, class Param, class = void>
struct check_invoke_kind {
  static constexpr invoke_kind value = invoke_kind::other;
};

template<class C, class Param>
struct check_invoke_kind<C, Param, typename std::enable_if<disjunction<std::is_same<C, Param>, std::is_base_of<C, Param>>::value>::type> {
  static constexpr invoke_kind value = invoke_kind::reference_to_object;
};

template<class C, class Param>
struct check_invoke_kind<C, Param, typename std::enable_if<is_reference_wrapper<Param>::value>::type> {
  static constexpr invoke_kind value = invoke_kind::reference_wrapper;
};

// member object pointer + reference to object
template<
    class T, class C, class U,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_to_object, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T C::* f, U&& u) noexcept -> decltype(static_cast<U&&>(u).*f)
{
  return static_cast<U&&>(u).*f;
}

// member object pointer + reference_wrapper
template<
    class T, class C, class U,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T C::* f, U&& u) noexcept -> decltype(static_cast<U&&>(u).get().*f)
{
  return static_cast<U&&>(u).get().*f;
}

// member object pointer + other(dereferenceable)
template<
    class T, class C, class U,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::other, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T C::* f, U&& u) noexcept -> decltype((*static_cast<U&&>(u)).*f)
{
  return (*static_cast<U&&>(u)).*f;
}

// member function pointer + reference to object
template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_to_object, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...), U&& u, Args&&... args) -> decltype((static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_to_object, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const, U&& u, Args&&... args) -> decltype((static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...);
}

#if __cpp_noexcept_function_type >= 201510L

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_to_object, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) noexcept, U&& u, Args&&... args) noexcept -> decltype((static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_to_object, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const noexcept, U&& u, Args&&... args) noexcept
    -> decltype((static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).*f)(static_cast<Args&&>(args)...);
}

#endif

// member function pointer + reference_wrapper to object
template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...), U&& u, Args&&... args) -> decltype((static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const, U&& u, Args&&... args) -> decltype((static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...);
}

#if __cpp_noexcept_function_type >= 201510L

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) noexcept, U&& u, Args&&... args) noexcept
    -> decltype((static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const noexcept, U&& u, Args&&... args) noexcept
    -> decltype((static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...))
{
  return (static_cast<U&&>(u).get().*f)(static_cast<Args&&>(args)...);
}

#endif

// member function pointer + other(dereferenceable)
template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::other, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...), U&& u, Args&&... args) -> decltype(((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...))
{
  return ((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::other, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const, U&& u, Args&&... args) -> decltype(((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...))
{
  return ((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...);
}

#if __cpp_noexcept_function_type >= 201510L

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::other, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) noexcept, U&& u, Args&&... args) noexcept -> decltype(((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...))
{
  return ((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...);
}

template<
    class T, class C, class... Params, class U, class... Args,
    typename std::enable_if<check_invoke_kind<C, typename remove_cvref<U>::type>::value == invoke_kind::other, std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(T (C::*f)(Params...) const noexcept, U&& u, Args&&... args) noexcept
    -> decltype(((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...))
{
  return ((*static_cast<U&&>(u)).*f)(static_cast<Args&&>(args)...);
}

#endif

template<class R>
struct invoke_r_impl {
  template<class F, class... Args>
  static constexpr R apply(F&& f, Args&&... args) noexcept(
      noexcept(detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...))
      && is_nothrow_convertible<decltype(detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...)), R>::value
  )
  {
    return detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...);
  }
};

template<>
struct invoke_r_impl<void> {
  template<class F, class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(F&& f, Args&&... args) noexcept(
      noexcept(detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...))
  )
  {
    static_cast<void>(detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...));
  }
};

template<class F, class, class... Args>
struct is_invocable_impl : false_type {};

template<class F, class... Args>
struct is_invocable_impl<F, void_t<decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...> : true_type {};

template<class F, class, class... Args>
struct is_nothrow_invocable_impl : false_type {};

template<class F, class... Args>
struct is_nothrow_invocable_impl<F, void_t<decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...>
    : bool_constant<noexcept(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))> {};

template<class R, class InvokeResult, class = void>
struct is_invocable_r_check : false_type {};

template<class R, class InvokeResult>
struct is_invocable_r_check<R, InvokeResult, typename std::enable_if<disjunction<std::is_void<R>, std::is_convertible<InvokeResult, R>>::value>::type>
    : true_type {};

template<class R, class F, class, class... Args>
struct is_invocable_r_impl : false_type {};

template<class R, class F, class... Args>
struct is_invocable_r_impl<R, F, void_t<decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...>
    : is_invocable_r_check<R, decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))> {};

template<class R, class InvokeResult>
struct is_nothrow_invocable_r_check : disjunction<std::is_void<R>, is_nothrow_convertible<InvokeResult, R>> {};

template<class R, class F, class, class... Args>
struct is_nothrow_invocable_r_impl : false_type {};

template<class R, class F, class... Args>
struct is_nothrow_invocable_r_impl<R, F, void_t<decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...>
    : conjunction<
          is_nothrow_invocable_r_check<R, decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>,
          bool_constant<noexcept(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>> {};

template<class F, class, class... Args>
struct invoke_result_impl {};

template<class F, class... Args>
struct invoke_result_impl<F, void_t<decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...> {
  using type = decltype(detail::invoke_impl(std::declval<F>(), std::declval<Args>()...));
};

}  // namespace detail

template<class F, class... Args>
struct is_invocable : detail::is_invocable_impl<F, void, Args...> {};

template<class R, class F, class... Args>
struct is_invocable_r : detail::is_invocable_r_impl<R, F, void, Args...> {};

template<class F, class... Args>
struct is_nothrow_invocable : detail::is_nothrow_invocable_impl<F, void, Args...> {};

template<class R, class F, class... Args>
struct is_nothrow_invocable_r : detail::is_nothrow_invocable_r_impl<R, F, void, Args...> {};

template<class F, class... Args>
struct invoke_result : detail::invoke_result_impl<F, void, Args...> {};

template<class F, class... Args>
constexpr typename invoke_result<F, Args...>::type invoke(F&& f, Args&&... args) noexcept(is_nothrow_invocable<F, Args...>::value)
{
  return detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...);
}

template<class R, class F, class... Args>
constexpr R invoke_r(F&& f, Args&&... args) noexcept(is_nothrow_invocable_r<R, F, Args...>::value)
{
  return detail::invoke_r_impl<R>::apply(static_cast<F&&>(f), static_cast<Args&&>(args)...);
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_INVOKE_HPP
