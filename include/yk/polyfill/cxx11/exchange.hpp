#ifndef YK_POLYFILL_CXX11_EXCHANGE_HPP
#define YK_POLYFILL_CXX11_EXCHANGE_HPP

#include <yk/polyfill/config.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class T, class U = T>
[[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T
exchange(T& obj, U&& new_value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_assignable<T, U>::value)
{
  T old_value = obj;
  obj = static_cast<U&&>(new_value);
  return old_value;
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_EXCHANGE_HPP
