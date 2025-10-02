#ifndef YK_POLYFILL_CXX11_IS_SWAPPABLE_WITH_HPP
#define YK_POLYFILL_CXX11_IS_SWAPPABLE_WITH_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>
#include <yk/polyfill/cxx11/void_t.hpp>

#include <utility>

namespace yk {

namespace polyfill {

namespace is_swappable_with_detail {

using std::swap;

template<class T, class U, class = void>
struct is_swappable_with_impl : false_type {};

template<class T, class U>
struct is_swappable_with_impl<T, U, void_t<decltype(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))>> : true_type {};

}  // namespace is_swappable_with_detail

template<class T, class U>
struct is_swappable_with : is_swappable_with_detail::is_swappable_with_impl<T, U> {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_IS_SWAPPABLE_WITH_HPP
