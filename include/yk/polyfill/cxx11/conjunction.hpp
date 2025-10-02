#ifndef YK_POLYFILL_CXX11_CONJUNCTION_HPP
#define YK_POLYFILL_CXX11_CONJUNCTION_HPP

#include <yk/polyfill/cxx11/integral_constant.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class... Traits>
struct conjunction;

template<>
struct conjunction<> : true_type {};

template<class Trait, class... Traits>
struct conjunction<Trait, Traits...> : std::conditional<Trait::value, conjunction<Traits...>, false_type>::type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_CONJUNCTION_HPP
