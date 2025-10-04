#ifndef YK_POLYFILL_CXX11_INVOKE_HPP
#define YK_POLYFILL_CXX11_INVOKE_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/cxx11/extension/specialization_of.hpp>

#include <yk/polyfill/cxx11/disjunction.hpp>
#include <yk/polyfill/cxx11/integral_constant.hpp>
#include <yk/polyfill/cxx11/remove_cvref.hpp>
#include <yk/polyfill/cxx11/void_t.hpp>

#include <functional>
#include <type_traits>

namespace yk {

namespace polyfill {

namespace invoke_detail {

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
struct check_invoke_kind<C, Param, typename std::enable_if<extension::is_specialization_of<Param, std::reference_wrapper>::value>::type> {
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

#if __cpp_noexcept_function_type >= 201510

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

#if __cpp_noexcept_function_type >= 201510

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

#if __cpp_noexcept_function_type >= 201510

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
  static constexpr R apply(F&& f, Args&&... args) noexcept(noexcept(invoke_detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...)))
  {
    return invoke_detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...);
  }
};

template<>
struct invoke_r_impl<void> {
  template<class F, class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(F&& f, Args&&... args) noexcept(noexcept(invoke_detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...)))
  {
    static_cast<void>(invoke_detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...));
  }
};

template<class F, class, class... Args>
struct is_invocable_impl : false_type {};

template<class F, class... Args>
struct is_invocable_impl<F, void_t<decltype(invoke_detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...> : true_type {};

template<class F, class, class... Args>
struct is_nothrow_invocable_impl : false_type {};

template<class F, class... Args>
struct is_nothrow_invocable_impl<F, void_t<decltype(invoke_detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...>
    : bool_constant<noexcept(invoke_detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))> {};

template<class R, class F, class, class... Args>
struct is_invocable_r_impl : false_type {};

template<class R, class F, class... Args>
struct is_invocable_r_impl<R, F, void_t<decltype(invoke_r_impl<R>::apply(std::declval<F>(), std::declval<Args>()...))>, Args...> : true_type {};

template<class R, class F, class, class... Args>
struct is_nothrow_invocable_r_impl : false_type {};

template<class R, class F, class... Args>
struct is_nothrow_invocable_r_impl<R, F, void_t<decltype(invoke_r_impl<R>::apply(std::declval<F>(), std::declval<Args>()...))>, Args...>
    : bool_constant<noexcept(invoke_r_impl<R>::apply(std::declval<F>(), std::declval<Args>()...))> {};

template<class F, class, class... Args>
struct invoke_result_impl {};

template<class F, class... Args>
struct invoke_result_impl<F, void_t<decltype(invoke_detail::invoke_impl(std::declval<F>(), std::declval<Args>()...))>, Args...> {
  using type = decltype(invoke_detail::invoke_impl(std::declval<F>(), std::declval<Args>()...));
};

}  // namespace invoke_detail

template<class F, class... Args>
struct is_invocable : invoke_detail::is_invocable_impl<F, void, Args...> {};

template<class R, class F, class... Args>
struct is_invocable_r : invoke_detail::is_invocable_r_impl<R, F, void, Args...> {};

template<class F, class... Args>
struct is_nothrow_invocable : invoke_detail::is_nothrow_invocable_impl<F, void, Args...> {};

template<class R, class F, class... Args>
struct is_nothrow_invocable_r : invoke_detail::is_nothrow_invocable_r_impl<R, F, void, Args...> {};

template<class F, class... Args>
struct invoke_result : invoke_detail::invoke_result_impl<F, void, Args...> {};

template<class F, class... Args>
constexpr typename invoke_result<F, Args...>::type invoke(F&& f, Args&&... args) noexcept(is_nothrow_invocable<F, Args...>::value)
{
  return invoke_detail::invoke_impl(static_cast<F&&>(f), static_cast<Args&&>(args)...);
}

template<class R, class F, class... Args>
constexpr R invoke_r(F&& f, Args&&... args) noexcept(is_nothrow_invocable_r<F, Args...>::value)
{
  return invoke_detail::invoke_r_impl<R>::apply(static_cast<F&&>(f), static_cast<Args&&>(args)...);
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_INVOKE_HPP
