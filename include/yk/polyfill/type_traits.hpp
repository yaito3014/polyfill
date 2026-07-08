#ifndef YK_ZZ_POLYFILL_TYPE_TRAITS_HPP
#define YK_ZZ_POLYFILL_TYPE_TRAITS_HPP

#include <yk/polyfill/bits/apply.hpp>
#include <yk/polyfill/bits/core_traits.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class T>
struct type_identity {
  using type = T;
};

template<class T>
struct is_bounded_array : false_type {};

template<class T, std::size_t N>
struct is_bounded_array<T[N]> : true_type {};

template<class T>
struct is_unbounded_array : false_type {};

template<class T>
struct is_unbounded_array<T[]> : true_type {};

template<class T>
struct is_null_pointer : false_type {};

template<>
struct is_null_pointer<std::nullptr_t> : true_type {};

template<>
struct is_null_pointer<std::nullptr_t const> : true_type {};

template<>
struct is_null_pointer<std::nullptr_t volatile> : true_type {};

template<>
struct is_null_pointer<std::nullptr_t const volatile> : true_type {};

namespace detail {

namespace swap_guard {

using std::swap;

template<class T, class U, class = void>
struct is_swappable_with_impl : false_type {};

template<class T, class U>
struct is_swappable_with_impl<T, U, void_t<decltype(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))>> : true_type {};

template<class T, class U, class = void>
struct is_nothrow_swappable_with_impl : false_type {};

template<class T, class U>
struct is_nothrow_swappable_with_impl<T, U, void_t<decltype(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))>>
    : bool_constant<noexcept(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))> {};

}  // namespace swap_guard

}  // namespace detail

template<class T, class U>
struct is_swappable_with : detail::swap_guard::is_swappable_with_impl<T, U> {};

template<class T, class U>
struct is_nothrow_swappable_with : detail::swap_guard::is_nothrow_swappable_with_impl<T, U> {};

namespace detail {

template<class T, class = void>
struct is_swappable_impl : false_type {};

template<class T>
struct is_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type> : is_swappable_with<T&, T&> {};

template<class T, class = void>
struct is_nothrow_swappable_impl : false_type {};

template<class T>
struct is_nothrow_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type>
    : is_nothrow_swappable_with<T&, T&> {};

}  // namespace detail

template<class T>
struct is_swappable : detail::is_swappable_impl<T> {};

template<class T>
struct is_nothrow_swappable : detail::is_nothrow_swappable_impl<T> {};

template<class F, class Tuple>
struct is_applicable : detail::is_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct is_nothrow_applicable : detail::is_nothrow_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct apply_result : detail::apply_result_impl<F, Tuple> {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_TYPE_TRAITS_HPP
