#ifndef YK_POLYFILL_EXTENSION_SPECIALIZATION_OF_HPP
#define YK_POLYFILL_EXTENSION_SPECIALIZATION_OF_HPP

#include <yk/polyfill/bits/core_traits.hpp>

namespace yk {

namespace polyfill {

namespace extension {

template<class T, template<class...> class TT>
struct is_specialization_of : false_type {};

template<template<class...> class TT, class... Ts>
struct is_specialization_of<TT<Ts...>, TT> : true_type {};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_SPECIALIZATION_OF_HPP
