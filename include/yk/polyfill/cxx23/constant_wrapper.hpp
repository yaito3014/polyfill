#ifndef YK_POLYFILL_CONSTANT_WRAPPER_HPP
#define YK_POLYFILL_CONSTANT_WRAPPER_HPP

#include <type_traits>

#if __cpp_lib_constant_wrapper >= 202506L
#define YK_POLYFILL_STD_HAS_CONSTANT_WRAPPER 1
#else
#define YK_POLYFILL_STD_HAS_CONSTANT_WRAPPER 0
#endif

#include <cstddef>

namespace yk {
namespace polyfill {

#if defined(YK_POLYFILL_NO_STD_CONSTANT_WRAPPER) || !(YK_POLYFILL_STD_HAS_CONSTANT_WRAPPER)

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

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_UNARY_OPERATOR(op)            \
  template<constexpr_param T>                                                    \
  friend constexpr auto operator op(T) noexcept -> constant_wrapper<+(T::value)> \
  {                                                                              \
    return {};                                                                   \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_BINARY_OPERATOR(op)                         \
  template<constexpr_param T, constexpr_param U>                                               \
  friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)> \
  {                                                                                            \
    return {};                                                                                 \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_LOGICAL_OPERATOR(op)                                                 \
  template<constexpr_param T, constexpr_param U>                                                                        \
    requires (!std::is_constructible_v<bool, decltype(T::value)> || !std::is_constructible_v<bool, decltype(U::value)>) \
  friend constexpr auto operator op(T, U) noexcept -> constant_wrapper<(T::value op U::value)>                          \
  {                                                                                                                     \
    return {};                                                                                                          \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_FIX_OPERATOR(op) \
  template<constexpr_param T>                                       \
  constexpr auto operator op(this T) noexcept                       \
    requires requires(T::value_type x) { op x; }                    \
  {                                                                 \
    return constant_wrapper<[] {                                    \
      auto c = T::value;                                            \
      return op c;                                                  \
    }()>{};                                                         \
  }                                                                 \
  template<constexpr_param T>                                       \
  constexpr auto operator op(this T, int) noexcept                  \
    requires requires(T::value_type x) { x op; }                    \
  {                                                                 \
    return constant_wrapper<[] {                                    \
      auto c = T::value;                                            \
      return c op;                                                  \
    }()>{};                                                         \
  }

#define YK_POLYFILL_CONSTANT_WRAPPER_DETAIL_DEFINE_ASSIGNMENT_OPERATOR(op) \
  template<constexpr_param T, constexpr_param R>                           \
  constexpr auto operator op(this T, R) noexcept                           \
    requires requires(T::value_type x) { x op R::value; }                  \
  {                                                                        \
    return constant_wrapper<[] {                                           \
      auto v = T::value;                                                   \
      return v op R::value;                                                \
    }()>{};                                                                \
  }

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

  template<constexpr_param T, constexpr_param... Args>
  constexpr auto operator()(this T, Args...) noexcept
    requires requires { constant_wrapper<T::value(Args::value...)>{}; }
  {
    return constant_wrapper<T::value(Args::value...)>{};
  }

  template<constexpr_param T, constexpr_param... Args>
  constexpr auto operator[](this T, Args...) noexcept -> constant_wrapper<(T::value[Args::value...])>
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
};

}  // namespace xo

template<xo::cw_fixed_value X, class>
struct constant_wrapper : public xo::cw_operators {
  static constexpr auto const& value = X.data;

  using type = constant_wrapper;
  using value_type = typename decltype(X)::type;

  template<xo::constexpr_param R>
  constexpr auto operator=(R) const noexcept
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

#else

using std::constant_wrapper;

#endif

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_CONSTANT_WRAPPER_HPP
