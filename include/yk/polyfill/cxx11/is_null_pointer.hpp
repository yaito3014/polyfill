#ifndef YK_POLYFILL_CXX11_IS_NULL_POINTER_HPP
#define YK_POLYFILL_CXX11_IS_NULL_POINTER_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <cstddef>

namespace yk {

namespace polyfill {

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

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_IS_NULL_POINTER_HPP
