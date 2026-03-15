#ifndef YK_POLYFILL_BITS_CONSTEXPR_SWAP_HPP
#define YK_POLYFILL_BITS_CONSTEXPR_SWAP_HPP

// constexpr-friendly swap: std::swap is not constexpr before C++20.

#include <yk/polyfill/config.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace detail {

template<class T>
YK_POLYFILL_CXX14_CONSTEXPR void constexpr_swap(T& a, T& b) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value)
{
  T tmp(static_cast<T&&>(a));
  a = static_cast<T&&>(b);
  b = static_cast<T&&>(tmp);
}

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_CONSTEXPR_SWAP_HPP
