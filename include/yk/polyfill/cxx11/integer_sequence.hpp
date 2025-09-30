#ifndef YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP
#define YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP

#include <utility>

#if __cpp_lib_integer_sequence >= 201304L
#define YK_POLYFILL_STD_HAS_INTEGER_SEQUENCE 1
#else
#define YK_POLYFILL_STD_HAS_INTEGER_SEQUENCE 0
#endif

#include <yk/polyfill/config.hpp>

#include <cstddef>

namespace yk::polyfill {

#if defined(YK_POLYFILL_NO_STD_INTEGER_SEQUENCE) || !(YK_POLYFILL_STD_HAS_INTEGER_SEQUENCE)

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

#else

using std::index_sequence;
using std::integer_sequence;
using std::index_sequence_for;

#endif

}  // namespace yk::polyfill

#endif  // YK_POLYFILL_CXX11_INTEGER_SEQUENCE_HPP
