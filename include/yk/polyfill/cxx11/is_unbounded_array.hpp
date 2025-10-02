#ifndef YK_POLYFILL_CXX11_IS_UNBOUNDED_ARRAY_HPP
#define YK_POLYFILL_CXX11_IS_UNBOUNDED_ARRAY_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

namespace yk {

namespace polyfill {

template<class T>
struct is_unbounded_array : false_type {};

template<class T>
struct is_unbounded_array<T[]> : true_type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_IS_UNBOUNDED_ARRAY_HPP
