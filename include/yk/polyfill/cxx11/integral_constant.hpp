#ifndef YK_POLYFILL_CXX11_INTEGRAL_CONSTANT_HPP
#define YK_POLYFILL_CXX11_INTEGRAL_CONSTANT_HPP

namespace yk {

namespace polyfill {

template<class T, T X>
struct integral_constant {
  static constexpr T value = X;

  using value_type = T;
  using type = integral_constant;

  constexpr operator value_type() const noexcept { return value; }
  constexpr value_type operator()() const noexcept { return value; }
};

template<bool X>
using bool_constant = integral_constant<bool, X>;

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_INTEGRAL_CONSTANT_HPP
