#ifndef YK_POLYFILL_CXX11_NEGATION_HPP
#define YK_POLYFILL_CXX11_NEGATION_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

namespace yk {

namespace polyfill {

template<class Trait>
struct negation : bool_constant<!Trait::value> {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_NEGATION_HPP
