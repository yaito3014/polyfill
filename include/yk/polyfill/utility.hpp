#ifndef YK_ZZ_POLYFILL_UTILITY_HPP
#define YK_ZZ_POLYFILL_UTILITY_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/config.hpp>

#include <type_traits>

#include <tuple>

#include <cstddef>

namespace yk {

namespace polyfill {

struct in_place_t {
  explicit in_place_t() = default;
};

YK_POLYFILL_INLINE constexpr in_place_t in_place{};

template<std::size_t I>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

#if __cpp_variable_templates >= 201304L

template<std::size_t I>
YK_POLYFILL_INLINE constexpr in_place_index_t<I> in_place_index{};

#endif

template<class T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

#if __cpp_variable_templates >= 201304L

template<class T>
YK_POLYFILL_INLINE constexpr in_place_type_t<T> in_place_type{};

#endif

template<class T, T... Is>
struct integer_sequence {
  using value_type = T;
  [[nodiscard]] static constexpr std::size_t size() noexcept { return sizeof...(Is); }
};

template<std::size_t... Is>
using index_sequence = integer_sequence<std::size_t, Is...>;

namespace detail {

template<class T, T N, T Current, class IntegerSequence>
struct make_integer_sequence_impl;

template<class T, T N, T... Is>
struct make_integer_sequence_impl<T, N, N, integer_sequence<T, Is...>> {
  using type = integer_sequence<T, Is...>;
};

template<class T, T N, T I, T... Is>
struct make_integer_sequence_impl<T, N, I, integer_sequence<T, Is...>> : make_integer_sequence_impl<T, N, I + 1, integer_sequence<T, Is..., I>> {};

}  // namespace detail

template<class T, T N>
using make_integer_sequence = typename detail::make_integer_sequence_impl<T, N, 0, integer_sequence<T>>::type;

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<class... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

namespace detail {

template<std::size_t I, class T, T... Is>
struct get_impl {};

template<class T, T Head, T... Rest>
struct get_impl<0, T, Head, Rest...> : integral_constant<T, Head> {};

template<std::size_t I, class T, T Head, T... Rest>
struct get_impl<I, T, Head, Rest...> : get_impl<I - 1, T, Rest...> {};

}  // namespace detail

template<std::size_t I, class T, T... Is>
constexpr T get(integer_sequence<T, Is...>) noexcept
{
  static_assert(I < sizeof...(Is), "I must be less than sizeof...(Is)");
  return detail::get_impl<I, T, Is...>::value;
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

// assume all C++20 features available
#if __cplusplus >= 202002L

namespace detail {

template<class X, class... Is>
struct subscript;

#if __cpp_multidimensional_subscript >= 202211L

template<class X, class... Is>
struct subscript {
  static constexpr auto value = X::value[Is::value...];
};

#else

// fallback specialization for pre-C++23
template<class X, class I>
struct subscript<X, I> {
  static constexpr auto value = X::value[I::value];
};

#endif

}  // namespace detail

namespace xo {

template<class T>
struct cw_fixed_value {
  T data;

  using type = T;
  constexpr cw_fixed_value(type v) noexcept : data(v) {}
};

template<class T, std::size_t Extent>
struct cw_fixed_value<T[Extent]> {
  T data[Extent];

  using type = T[Extent];
  constexpr cw_fixed_value(T (&arr)[Extent]) noexcept
  {
    for (std::size_t i = 0; i < Extent; ++i) {
      data[i] = arr[i];
    }
  }
};

template<class T, std::size_t Extent>
cw_fixed_value(T (&)[Extent]) -> cw_fixed_value<T[Extent]>;

}  // namespace xo

template<xo::cw_fixed_value X, class = typename decltype(xo::cw_fixed_value(X))::type>
struct constant_wrapper;

namespace xo {

template<class T>
concept constexpr_param = requires { typename constant_wrapper<T::value>; };

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(op)                          \
  template<constexpr_param T>                                                                  \
  [[nodiscard]] friend constexpr auto operator op(T) noexcept -> constant_wrapper<+(T::value)> \
  {                                                                                            \
    return {};                                                                                 \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(op)                                       \
  template<constexpr_param T, constexpr_param U>                                                             \
  [[nodiscard]] friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)> \
  {                                                                                                          \
    return {};                                                                                               \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR(op)                                                 \
  template<constexpr_param T, constexpr_param U>                                                                        \
    requires (!std::is_constructible_v<bool, decltype(T::value)> || !std::is_constructible_v<bool, decltype(U::value)>) \
  [[nodiscard]] friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)>            \
  {                                                                                                                     \
    return {};                                                                                                          \
  }

#if __cpp_explicit_this_parameter >= 202110L

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_FIX_OPERATOR(op) \
  template<constexpr_param T>                                       \
  [[nodiscard]] constexpr auto operator op(this T) noexcept         \
    requires requires(T::value_type x) { op x; }                    \
  {                                                                 \
    return constant_wrapper<[] {                                    \
      auto c = T::value;                                            \
      return op c;                                                  \
    }()>{};                                                         \
  }                                                                 \
  template<constexpr_param T>                                       \
  [[nodiscard]] constexpr auto operator op(this T, int) noexcept    \
    requires requires(T::value_type x) { x op; }                    \
  {                                                                 \
    return constant_wrapper<[] {                                    \
      auto c = T::value;                                            \
      return c op;                                                  \
    }()>{};                                                         \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(op) \
  template<constexpr_param T, constexpr_param R>                           \
  [[nodiscard]] constexpr auto operator op(this T, R) noexcept             \
    requires requires(T::value_type x) { x op R::value; }                  \
  {                                                                        \
    return constant_wrapper<[] {                                           \
      auto v = T::value;                                                   \
      return v op R::value;                                                \
    }()>{};                                                                \
  }

#endif

struct cw_operators {
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(+)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(-)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(~)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(!)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(&)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(*)

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(+)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(-)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(*)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(/)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(%)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<<)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(>>)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(&)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(|)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(^)

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<=>)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(==)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(!=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(>)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(>=)

  template<constexpr_param T, constexpr_param U>
  friend constexpr auto operator,(T, U) noexcept = delete;

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(->*)

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR(&&)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR(||)

#if __cpp_explicit_this_parameter >= 202110L

  template<constexpr_param T, constexpr_param... Args>
  [[nodiscard]] constexpr auto operator()(this T, Args...) noexcept
    requires requires { constant_wrapper<T::value(Args::value...)>{}; }
  {
    return constant_wrapper<T::value(Args::value...)>{};
  }

  template<constexpr_param T, constexpr_param... Args>
  [[nodiscard]] constexpr auto operator[](this T, Args...) noexcept -> constant_wrapper<detail::subscript<T, Args...>::value>
  {
    return {};
  }

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_FIX_OPERATOR(++)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_FIX_OPERATOR(--)

  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(+=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(-=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(*=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(/=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(&=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(|=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(^=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(<<=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(>>=)

#endif
};

#undef YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR
#undef YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_FIX_OPERATOR
#undef YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR
#undef YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR
#undef YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR

}  // namespace xo

template<xo::cw_fixed_value X, class>
struct constant_wrapper : public xo::cw_operators {
  static constexpr auto const& value = X.data;

  using type = constant_wrapper;
  using value_type = typename decltype(X)::type;

  template<xo::constexpr_param R>
  [[nodiscard]] constexpr auto operator=(R) const noexcept
    requires requires(value_type x) { x = R::value; }
  {
    return constant_wrapper<[] {
      auto v = value;
      return v = R::value;
    }()>{};
  }

  constexpr operator decltype(auto)() const noexcept { return value; }
};

template<xo::cw_fixed_value X>
inline constexpr auto cw = constant_wrapper<X>{};

#endif

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

#endif  // YK_ZZ_POLYFILL_UTILITY_HPP
