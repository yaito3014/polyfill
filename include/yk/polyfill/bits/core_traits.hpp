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

template<class Trait>
struct conjunction<Trait> : Trait {};

template<class Trait, class Next, class... Traits>
struct conjunction<Trait, Next, Traits...> : std::conditional<bool(Trait::value), conjunction<Next, Traits...>, Trait>::type {};

template<class... Traits>
struct disjunction;

template<>
struct disjunction<> : false_type {};

template<class Trait>
struct disjunction<Trait> : Trait {};

template<class Trait, class Next, class... Traits>
struct disjunction<Trait, Next, Traits...> : std::conditional<bool(Trait::value), Trait, disjunction<Next, Traits...>>::type {};

namespace detail {

// The hypothetical function of the LWG 4028 proposed resolution, which words the conversion as initialization of a parameter
// rather than of a returned object as [meta.rel] does, so the destructor of To counts too, as in the major implementations
// (LWG 3400). To of array or function type would be adjusted to a pointer here, but std::is_convertible already rejects them.
template<class To>
void conv_dest(To) noexcept;

}  // namespace detail

template<class From, class To, class = void>
struct is_nothrow_convertible : false_type {};

template<class From, class To>
struct is_nothrow_convertible<From, To, typename std::enable_if<std::is_convertible<From, To>::value && !std::is_void<To>::value>::type>
    : bool_constant<noexcept(detail::conv_dest<To>(std::declval<From>()))> {};

template<class From, class To>
struct is_nothrow_convertible<From, To, typename std::enable_if<std::is_void<From>::value && std::is_void<To>::value>::type> : true_type {};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_CORE_TRAITS_HPP
