#ifndef YK_ZZ_POLYFILL_BITS_SWAP_HPP
#define YK_ZZ_POLYFILL_BITS_SWAP_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace detail {

template<class T, class = void>
struct has_adl_swap : false_type {};

template<class T>
struct has_adl_swap<T, void_t<decltype(swap(std::declval<T&>(), std::declval<T&>()))>> : true_type {};

template<class T, typename std::enable_if<has_adl_swap<T>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR void constexpr_swap(T& a, T& b) noexcept(noexcept(swap(a, b)))
{
  swap(a, b);
}

template<class T, typename std::enable_if<!has_adl_swap<T>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR void constexpr_swap(T& a, T& b) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value)
{
  T tmp(std::move(a));
  a = std::move(b);
  b = std::move(tmp);
}

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_SWAP_HPP
