#ifndef YK_POLYFILL_CXX11_IS_BOUNDED_ARRAY_HPP
#define YK_POLYFILL_CXX11_IS_BOUNDED_ARRAY_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <cstddef>

namespace yk {

namespace polyfill {

template<class T>
struct is_bounded_array : false_type {};

template<class T, std::size_t N>
struct is_bounded_array<T[N]> : true_type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_IS_BOUNDED_ARRAY_HPP
