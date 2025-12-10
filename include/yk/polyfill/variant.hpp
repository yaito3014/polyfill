#ifndef YK_POLYFILL_VARIANT_HPP
#define YK_POLYFILL_VARIANT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/extension/pack_indexing.hpp>

#include <cstddef>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace yk {

namespace polyfill {

namespace variant_detail {

template<class... Ts>
struct variadic_union {};

template<class Head, class... Rest>
struct variadic_union<Head, Rest...> {
  union {
    Head head;
    variadic_union<Rest...> rest;
  };
};

}  // namespace variant_detail

template<class... Ts>
class variant;

template<class T>
struct variant_size;

template<class T>
struct variant_size<T const> : variant_size<T> {};

template<class... Ts>
struct variant_size<variant<Ts...>> : integral_constant<std::size_t, sizeof...(Ts)> {};

template<std::size_t I, class T>
struct variant_alternative;

template<std::size_t I, class T>
struct variant_alternative<I, T const> : variant_alternative<I, T> {};

template<std::size_t I, class... Ts>
struct variant_alternative<I, variant<Ts...>> : extension::pack_indexing<I, Ts...> {};

YK_POLYFILL_INLINE constexpr std::size_t variant_npos = -1;

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_VARIANT_HPP
