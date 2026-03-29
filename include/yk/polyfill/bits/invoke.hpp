#ifndef YK_POLYFILL_BITS_INVOKE_HPP
#define YK_POLYFILL_BITS_INVOKE_HPP

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
constexpr auto invoke_impl(T C::* f, U&& u) noexcept(noexcept((*static_cast<U&&>(u)).*f)) -> decltype((*static_cast<U&&>(u)).*f)
{
  return (*static_cast<U&&>(u)).*f;
}

template<class MFP>
struct get_class_from_member_function_pointer {};

#define YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP(cv_opt, ref_opt, noexcept_opt, variadic_opt)                      \
  template<class T, class C, class... Params>                                                                   \
  struct get_class_from_member_function_pointer<T (C::*)(Params..., variadic_opt) cv_opt ref_opt noexcept_opt> { \
    using type = C;                                                                                             \
  };

#define YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV(ref_opt, noexcept_opt, variadic_opt)  \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP(, ref_opt, noexcept_opt, variadic_opt)         \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP(const, ref_opt, noexcept_opt, variadic_opt)    \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP(volatile, ref_opt, noexcept_opt, variadic_opt) \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP(const volatile, ref_opt, noexcept_opt, variadic_opt)

#define YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF(noexcept_opt, variadic_opt) \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV(, noexcept_opt, variadic_opt)         \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV(&, noexcept_opt, variadic_opt)        \
  YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV(&&, noexcept_opt, variadic_opt)

// non-variadic, non-noexcept
YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF(, )

// variadic, non-noexcept
YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF(, ...)

#if __cpp_noexcept_function_type >= 201510L

// non-variadic, noexcept
YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF(noexcept, )

// variadic, noexcept
YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF(noexcept, ...)

#endif

#undef YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV_REF
#undef YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP_CV
#undef YK_POLYFILL_DETAIL_GET_CLASS_FROM_MFP

// member function pointer + reference to object
template<
    class MFP, class U, class... Args, typename std::enable_if<std::is_member_function_pointer<MFP>::value, std::nullptr_t>::type = nullptr,
    typename std::enable_if<
        check_invoke_kind<typename get_class_from_member_function_pointer<MFP>::type, typename remove_cvref<U>::type>::value
            == invoke_kind::reference_to_object,
        std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(MFP mfp, U&& u, Args&&... args) noexcept(noexcept((std::forward<U>(u).*mfp)(std::forward<Args>(args)...)))
    -> decltype((std::forward<U>(u).*mfp)(std::forward<Args>(args)...))
{
  return (std::forward<U>(u).*mfp)(std::forward<Args>(args)...);
}

// member function pointer + reference_wrapper
template<
    class MFP, class U, class... Args, typename std::enable_if<std::is_member_function_pointer<MFP>::value, std::nullptr_t>::type = nullptr,
    typename std::enable_if<
        check_invoke_kind<typename get_class_from_member_function_pointer<MFP>::type, typename remove_cvref<U>::type>::value == invoke_kind::reference_wrapper,
        std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(MFP mfp, U&& u, Args&&... args) noexcept(noexcept((std::forward<U>(u).get().*mfp)(std::forward<Args>(args)...)))
    -> decltype((std::forward<U>(u).get().*mfp)(std::forward<Args>(args)...))
{
  return (std::forward<U>(u).get().*mfp)(std::forward<Args>(args)...);
}

// member function pointer + other(dereferenceable)
template<
    class MFP, class U, class... Args, typename std::enable_if<std::is_member_function_pointer<MFP>::value, std::nullptr_t>::type = nullptr,
    typename std::enable_if<
        check_invoke_kind<typename get_class_from_member_function_pointer<MFP>::type, typename remove_cvref<U>::type>::value == invoke_kind::other,
        std::nullptr_t>::type = nullptr>
constexpr auto invoke_impl(MFP mfp, U&& u, Args&&... args) noexcept(noexcept(((*std::forward<U>(u)).*mfp)(std::forward<Args>(args)...)))
    -> decltype(((*std::forward<U>(u)).*mfp)(std::forward<Args>(args)...))
{
  return ((*std::forward<U>(u)).*mfp)(std::forward<Args>(args)...);
}

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

#endif  // YK_POLYFILL_BITS_INVOKE_HPP
