#ifndef YK_ZZ_POLYFILL_BITS_TYPE_TRAITS_HPP
#define YK_ZZ_POLYFILL_BITS_TYPE_TRAITS_HPP

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

template<class To>
void is_nothrow_convertible_test(To) noexcept;

}  // namespace detail

template<class From, class To, class = void>
struct is_nothrow_convertible : false_type {};

template<class From, class To>
struct is_nothrow_convertible<From, To, typename std::enable_if<std::is_convertible<From, To>::value>::type>
    : bool_constant<noexcept(detail::is_nothrow_convertible_test<To>(std::declval<From>()))> {};

template<>
struct is_nothrow_convertible<void, void> : true_type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_TYPE_TRAITS_HPP
