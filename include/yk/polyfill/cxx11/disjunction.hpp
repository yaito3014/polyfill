#ifndef YK_POLYFILL_CXX11_DISJUNCTION_HPP
#define YK_POLYFILL_CXX11_DISJUNCTION_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class... Traits>
struct disjunction;

template<>
struct disjunction<> : false_type {};

template<class Trait, class... Traits>
struct disjunction<Trait, Traits...> : std::conditional<Trait::value, true_type, disjunction<Traits...>>::type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_DISJUNCTION_HPP
