#ifndef YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP
#define YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP

#include <cstddef>

namespace yk {

namespace polyfill {

template<class T, T... Is>
struct integer_sequence {
  using value_type = T;
  [[nodiscard]] static constexpr std::size_t size() noexcept { return sizeof...(Is); }
};

template<std::size_t... Is>
using index_sequence = integer_sequence<std::size_t, Is...>;

namespace integer_sequence_detail {

template<class T, T N, T Current, class IntegerSequence>
struct make_integer_sequence_impl;

template<class T, T N, T... Is>
struct make_integer_sequence_impl<T, N, N, integer_sequence<T, Is...>> {
  using type = integer_sequence<T, Is...>;
};

template<class T, T N, T I, T... Is>
struct make_integer_sequence_impl<T, N, I, integer_sequence<T, Is...>> : make_integer_sequence_impl<T, N, I + 1, integer_sequence<T, Is..., I>> {};

}  // namespace integer_sequence_detail

template<class T, T N>
using make_integer_sequence = typename integer_sequence_detail::make_integer_sequence_impl<T, N, 0, integer_sequence<T>>::type;

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<class... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP
