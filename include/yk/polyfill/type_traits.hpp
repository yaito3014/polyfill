#ifndef YK_ZZ_POLYFILL_TYPE_TRAITS_HPP
#define YK_ZZ_POLYFILL_TYPE_TRAITS_HPP

#include <yk/polyfill/bits/apply.hpp>
#include <yk/polyfill/bits/core_traits.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

template<class T>
struct type_identity {
  using type = T;
};

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

namespace detail {

namespace swap_guard {

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

}  // namespace swap_guard

}  // namespace detail

template<class T, class U>
struct is_swappable_with : detail::swap_guard::is_swappable_with_impl<T, U> {};

template<class T, class U>
struct is_nothrow_swappable_with : detail::swap_guard::is_nothrow_swappable_with_impl<T, U> {};

namespace detail {

template<class T, class = void>
struct is_swappable_impl : false_type {};

template<class T>
struct is_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type> : is_swappable_with<T&, T&> {};

template<class T, class = void>
struct is_nothrow_swappable_impl : false_type {};

template<class T>
struct is_nothrow_swappable_impl<T, typename std::enable_if<disjunction<std::is_object<T>, std::is_reference<T>>::value>::type>
    : is_nothrow_swappable_with<T&, T&> {};

}  // namespace detail

template<class T>
struct is_swappable : detail::is_swappable_impl<T> {};

template<class T>
struct is_nothrow_swappable : detail::is_nothrow_swappable_impl<T> {};

template<class F, class Tuple>
struct is_applicable : detail::is_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct is_nothrow_applicable : detail::is_nothrow_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct apply_result : detail::apply_result_impl<F, Tuple> {};

// constant_wrapper core requires C++17 (auto NTTP); richer pieces are gated on newer
// features individually (concepts, three-way comparison, explicit object parameters).
#if __cplusplus >= 201703L

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

template<auto X, class = decltype(X)>
struct constant_wrapper;

namespace xo {

template<class T, class = void>
struct is_constexpr_param : std::false_type {};

template<class T>
struct is_constexpr_param<T, std::void_t<constant_wrapper<T::value>>> : std::true_type {};

#if __cpp_concepts >= 201907L
template<class T>
concept constexpr_param = is_constexpr_param<T>::value;
#endif

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(op)                                      \
  template<class T, typename std::enable_if<is_constexpr_param<T>::value, std::nullptr_t>::type = nullptr> \
  [[nodiscard]] friend constexpr auto operator op(T) noexcept -> constant_wrapper<(op T::value)>           \
  {                                                                                                        \
    return {};                                                                                             \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(op)                                       \
  template<class T, class U,                                                                                 \
           typename std::enable_if<is_constexpr_param<T>::value && is_constexpr_param<U>::value,             \
                                   std::nullptr_t>::type = nullptr>                                          \
  [[nodiscard]] friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)> \
  {                                                                                                          \
    return {};                                                                                               \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR(op)                                      \
  template<class T, class U,                                                                                 \
           typename std::enable_if<is_constexpr_param<T>::value && is_constexpr_param<U>::value              \
                                       && (!std::is_constructible<bool, decltype(T::value)>::value           \
                                           || !std::is_constructible<bool, decltype(U::value)>::value),      \
                                   std::nullptr_t>::type = nullptr>                                          \
  [[nodiscard]] friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)> \
  {                                                                                                          \
    return {};                                                                                               \
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

#if __cpp_impl_three_way_comparison >= 201907L
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<=>)
#endif
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(<=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(==)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(!=)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(>)
  YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(>=)

  template<class T, class U>
  friend constexpr auto operator,(T, U) noexcept
      -> typename std::enable_if<is_constexpr_param<T>::value && is_constexpr_param<U>::value>::type = delete;

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

template<auto X, class T>
struct constant_wrapper : public xo::cw_operators {
  static constexpr decltype(auto) value = (X);

  using type = constant_wrapper;
  using value_type = decltype(X);

  static_assert(std::is_same<T, value_type>::value, "constant_wrapper's second template argument must be its value_type");

  // Uses a lambda in a template-argument to compute the assigned value, so it requires C++20.
#if __cplusplus >= 202002L
  template<xo::constexpr_param R>
  [[nodiscard]] constexpr auto operator=(R) const noexcept
    requires requires(value_type x) { x = R::value; }
  {
    return constant_wrapper<[] {
      auto v = value;
      return v = R::value;
    }()>{};
  }
#endif

  constexpr operator decltype(auto)() const noexcept { return value; }
};

template<auto X>
inline constexpr auto cw = constant_wrapper<X>{};

#endif

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_TYPE_TRAITS_HPP
