#ifndef YK_ZZ_POLYFILL_BITS_CORE_TRAITS_HPP
#define YK_ZZ_POLYFILL_BITS_CORE_TRAITS_HPP

#include <type_traits>

namespace yk {

namespace polyfill {

template<class... Ts>
using void_t = void;

template<class T>
struct remove_cvref {
  using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

template<class T, T X>
struct integral_constant {
  static constexpr T value = X;

  using value_type = T;
  using type = integral_constant;

  constexpr operator value_type() const noexcept { return value; }
  [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
};

template<bool X>
using bool_constant = integral_constant<bool, X>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template<class Trait>
struct negation : bool_constant<!Trait::value> {};

template<class... Traits>
struct conjunction;

template<>
struct conjunction<> : true_type {};

template<class Trait, class... Traits>
struct conjunction<Trait, Traits...> : std::conditional<Trait::value, conjunction<Traits...>, false_type>::type {};

template<class... Traits>
struct disjunction;

template<>
struct disjunction<> : false_type {};

template<class Trait, class... Traits>
struct disjunction<Trait, Traits...> : std::conditional<Trait::value, true_type, disjunction<Traits...>>::type {};

namespace detail {

// Tests the conversion as initialization of a parameter rather than of a returned object as [meta.rel] words it,
// so the destructor of To counts too. This matches the major implementations (LWG 3400) and the direction of LWG 4028.
// To of array or function type would be adjusted to a pointer here, but std::is_convertible already rejects them.
template<class To>
void is_nothrow_convertible_test(To) noexcept;

}  // namespace detail

template<class From, class To, class = void>
struct is_nothrow_convertible : false_type {};

template<class From, class To>
struct is_nothrow_convertible<From, To, typename std::enable_if<std::is_convertible<From, To>::value && !std::is_void<To>::value>::type>
    : bool_constant<noexcept(detail::is_nothrow_convertible_test<To>(std::declval<From>()))> {};

template<class From, class To>
struct is_nothrow_convertible<From, To, typename std::enable_if<std::is_void<From>::value && std::is_void<To>::value>::type> : true_type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_CORE_TRAITS_HPP
