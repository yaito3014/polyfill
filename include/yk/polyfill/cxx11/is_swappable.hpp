#ifndef YK_POLYFILL_CXX11_IS_SWAPPABLE_HPP
#define YK_POLYFILL_CXX11_IS_SWAPPABLE_HPP

#include <yk/polyfill/cxx11/disjunction.hpp>
#include <yk/polyfill/cxx11/integral_constant.hpp>
#include <yk/polyfill/cxx11/is_swappable_with.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace is_swappable_detail {

template<class T, class = void>
struct is_swappable_impl : false_type {};

template<class T>
struct is_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type> : is_swappable_with<T&, T&> {};

}  // namespace is_swappable_detail

template<class T>
struct is_swappable : is_swappable_detail::is_swappable_impl<T> {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_IS_SWAPPABLE_HPP
