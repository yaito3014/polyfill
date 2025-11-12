#ifndef YK_POLYFILL_TYPE_TRAITS_HPP
#define YK_POLYFILL_TYPE_TRAITS_HPP

#include <yk/polyfill/bits/apply_detail.hpp>
#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/extension/always_false.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class T>
struct is_bounded_array : false_type {};

template<class T, std::size_t N>
struct is_bounded_array<T[N]> : true_type {};

template<class T>
struct is_unbounded_array : false_type {};

template<class T>
struct is_unbounded_array<T[]> : true_type {};

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

namespace is_swappable_with_detail {

using std::swap;

template<class T, class U, class = void>
struct is_swappable_with_impl : false_type {};

template<class T, class U>
struct is_swappable_with_impl<T, U, void_t<decltype(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))>> : true_type {};

template<class T, class U, class = void>
struct is_nothrow_swappable_with_impl : false_type {};

template<class T, class U>
struct is_nothrow_swappable_with_impl<T, U, void_t<decltype(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))>>
    : bool_constant<noexcept(swap(std::declval<T>(), std::declval<U>()), swap(std::declval<U>(), std::declval<T>()))> {};

}  // namespace is_swappable_with_detail

template<class T, class U>
struct is_swappable_with : is_swappable_with_detail::is_swappable_with_impl<T, U> {};

template<class T, class U>
struct is_nothrow_swappable_with : is_swappable_with_detail::is_nothrow_swappable_with_impl<T, U> {};

namespace is_swappable_detail {

template<class T, class = void>
struct is_swappable_impl : false_type {};

template<class T>
struct is_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type> : is_swappable_with<T&, T&> {};

template<class T, class = void>
struct is_nothrow_swappable_impl : false_type {};

template<class T>
struct is_nothrow_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type>
    : is_nothrow_swappable_with<T&, T&> {};

}  // namespace is_swappable_detail

template<class T>
struct is_swappable : is_swappable_detail::is_swappable_impl<T> {};

template<class T>
struct is_nothrow_swappable : is_swappable_detail::is_nothrow_swappable_impl<T> {};

template<class F, class Tuple>
struct is_applicable : apply_detail::is_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct is_nothrow_applicable : apply_detail::is_nothrow_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct apply_result : apply_detail::apply_result_impl<F, Tuple> {};

namespace reference_from_temporary_detail {

template<class T>
[[noreturn]] T VAL() noexcept
{
  static_assert(extension::always_false<T>::value, "do not call VAL not in unevaluated context");
}

template<class T, class U, class = void>
struct is_strictly_constructible : false_type {};

template<class T, class U>
struct is_strictly_constructible<T, U, void_t<decltype(T(VAL<U>()))>> : true_type {};

template<class T, class U, class = void>
struct is_strictly_convertible : false_type {};

template<class T, class U>
struct is_strictly_convertible<T, U, void_t<decltype(std::declval<void (&)(T)>()(VAL<U>()))>> : true_type {};

template<class T, class U>
struct is_lifetime_extended : false_type {};

template<class T, class U>
struct is_lifetime_extended<T const&, U> : std::is_same<U, typename std::remove_reference<U>::type> {};

template<class T, class U>
struct is_lifetime_extended<T&&, U> : std::is_same<U, typename std::remove_reference<U>::type> {};

}  // namespace reference_from_temporary_detail

template<class T, class U>
struct reference_constructs_from_temporary
    : conjunction<
          std::is_reference<T>, reference_from_temporary_detail::is_strictly_constructible<T, U>, reference_from_temporary_detail::is_lifetime_extended<T, U>> {
};

template<class T, class U>
struct reference_converts_from_temporary
    : conjunction<
          std::is_reference<T>, reference_from_temporary_detail::is_strictly_convertible<T, U>, reference_from_temporary_detail::is_lifetime_extended<T, U>> {};

// assume all C++20 features available
#if __cplusplus >= 202002L

namespace constant_wrapper_detail {

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

}  // namespace constant_wrapper_detail

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
  [[nodiscard]] constexpr auto operator[](this T, Args...) noexcept -> constant_wrapper<constant_wrapper_detail::subscript<T, Args...>::value>
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

#endif  // YK_POLYFILL_TYPE_TRAITS_HPP
