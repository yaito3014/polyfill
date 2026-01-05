#ifndef YK_POLYFILL_UTILITY_HPP
#define YK_POLYFILL_UTILITY_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/config.hpp>

#include <tuple>

#include <cstddef>

namespace yk {

namespace polyfill {

struct in_place_t {
  explicit in_place_t() = default;
};

YK_POLYFILL_INLINE constexpr in_place_t in_place{};

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

namespace integer_sequence_detail {

template<std::size_t I, class T, T... Is>
struct get_impl {};

template<class T, T Head, T... Rest>
struct get_impl<0, T, Head, Rest...> : integral_constant<T, Head> {};

template<std::size_t I, class T, T Head, T... Rest>
struct get_impl<I, T, Head, Rest...> : get_impl<I - 1, T, Rest...> {};

}  // namespace integer_sequence_detail

template<std::size_t I, class T, T... Is>
constexpr T get(integer_sequence<T, Is...>) noexcept
{
  static_assert(I < sizeof...(Is), "I must be less than sizeof...(Is)");
  return integer_sequence_detail::get_impl<I, T, Is...>::value;
}

template<class T, class U = T>
[[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T
exchange(T& obj, U&& new_value) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_assignable<T, U>::value)
{
  T old_value = obj;
  obj = static_cast<U&&>(new_value);
  return old_value;
}

template<class T>
constexpr typename std::add_const<T>::type& as_const(T& x) noexcept
{
  return x;
}

template<class T>
void as_const(T const&&) = delete;

}  // namespace polyfill

}  // namespace yk

template<class T, T... Is>
struct std::tuple_size<yk::polyfill::integer_sequence<T, Is...>> : yk::polyfill::integral_constant<std::size_t, sizeof...(Is)> {};

template<std::size_t I, class T, T... Is>
struct std::tuple_element<I, yk::polyfill::integer_sequence<T, Is...>> {
  static_assert(I < sizeof...(Is), "I must be less than sizeof...(Is)");
  using type = T;
};

template<std::size_t I, class T, T... Is>
struct std::tuple_element<I, yk::polyfill::integer_sequence<T, Is...> const> {
  static_assert(I < sizeof...(Is), "I must be less than sizeof...(Is)");
  using type = T;
};

#endif  // YK_POLYFILL_UTILITY_HPP
