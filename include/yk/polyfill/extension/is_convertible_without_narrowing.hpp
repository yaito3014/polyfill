#ifndef YK_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP
#define YK_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace extension {

namespace is_convertible_without_narrowing_detail {

template<template<class> class F, class... Ts>
struct apply_disjunction_expand_types : disjunction<F<Ts>...> {};

template<template<class> class... Fs>
struct apply_disjunction {
  template<class... Ts>
  struct apply : disjunction<apply_disjunction_expand_types<Fs, Ts...>...> {};
};

template<class From, class To, class = void>
struct is_convertible_without_narrowing_impl : apply_disjunction<std::is_void, std::is_reference, std::is_function, is_unbounded_array>::apply<
                                                   typename std::remove_cv<From>::type, typename std::remove_cv<To>::type> {};

template<class From, class To>
struct is_convertible_without_narrowing_impl<
    From, To, typename std::enable_if<std::is_same<decltype(typename type_identity<To[]>::type{std::declval<From>()}), To[1]>::value>::type> : true_type {};

}  // namespace is_convertible_without_narrowing_detail

template<class From, class To>
struct is_convertible_without_narrowing
    : conjunction<std::is_convertible<From, To>, is_convertible_without_narrowing_detail::is_convertible_without_narrowing_impl<From, To>> {};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP
