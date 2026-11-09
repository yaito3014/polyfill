#ifndef YK_ZZ_POLYFILL_CONCEPTS_HPP
#define YK_ZZ_POLYFILL_CONCEPTS_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/type_traits.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace detail {

// [concept.swappable] ranges::swap, with each "if ... otherwise" step of its specification
// expressed as a trait so the call is SFINAE-friendly before C++20.
namespace ranges_swap {

// Poison pill: with it in scope, std::swap's generic overload and unconstrained
// program-defined swap templates tie with it, so only more specialized overloads are selected.
template<class T>
void swap(T&, T&) = delete;

template<class T>
struct is_class_or_enum {
  using type = typename std::remove_reference<T>::type;
  static constexpr bool value = std::is_class<type>::value || std::is_union<type>::value || std::is_enum<type>::value;
};

enum class strategy { none, adl, array, exchange };

// The class/enum test comes first so the swap call is never formed otherwise: looking up swap
// for a pointer would instantiate a pointee class template, which may be ill-formed.
template<class T, class U, bool = is_class_or_enum<T>::value || is_class_or_enum<U>::value, class = void>
struct adl_swappable : false_type {};

template<class T, class U>
struct adl_swappable<T, U, true, void_t<decltype(swap(std::declval<T>(), std::declval<U>()))>> : true_type {};

template<class T, class U>
struct strategy_of;

template<class T, class U>
struct array_swappable : false_type {};

template<class T, class U, std::size_t N>
struct array_swappable<T (&)[N], U (&)[N]> : bool_constant<strategy_of<T&, U&>::value != strategy::none> {};

// move_constructible<T> && assignable_from<T&, T>
template<class T, class = void>
struct exchangeable : false_type {};

template<class T>
struct exchangeable<T, typename std::enable_if<std::is_same<decltype(std::declval<T&>() = std::declval<T>()), T&>::value>::type>
    : bool_constant<std::is_nothrow_destructible<T>::value && std::is_constructible<T, T>::value && std::is_convertible<T, T>::value> {};

template<class T, class U>
struct exchange_swappable : false_type {};

template<class T>
struct exchange_swappable<T&, T&> : exchangeable<T> {};

template<class T, class U>
struct strategy_of : std::integral_constant<
                         strategy, adl_swappable<T, U>::value        ? strategy::adl
                                   : array_swappable<T, U>::value    ? strategy::array
                                   : exchange_swappable<T, U>::value ? strategy::exchange
                                                                     : strategy::none> {};

template<class T, class U, strategy = strategy_of<T, U>::value>
struct nothrow_swappable : false_type {};

template<class T, class U>
struct nothrow_swappable<T, U, strategy::adl> : bool_constant<noexcept(swap(std::declval<T>(), std::declval<U>()))> {};

template<class T, class U, std::size_t N>
struct nothrow_swappable<T (&)[N], U (&)[N], strategy::array> : nothrow_swappable<T&, U&> {};

template<class T>
struct nothrow_swappable<T&, T&, strategy::exchange>
    : bool_constant<std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value> {};

template<strategy S>
using strategy_tag = std::integral_constant<strategy, S>;

template<class T, class U>
YK_POLYFILL_CXX14_CONSTEXPR void swap_impl(strategy_tag<strategy::adl>, T&& t, U&& u) noexcept(nothrow_swappable<T, U>::value)
{
  (void)swap(std::forward<T>(t), std::forward<U>(u));
}

template<class T>
YK_POLYFILL_CXX14_CONSTEXPR void swap_impl(strategy_tag<strategy::exchange>, T& a, T& b) noexcept(nothrow_swappable<T&, T&>::value)
{
  T tmp(std::move(a));
  a = std::move(b);
  b = std::move(tmp);
}

template<class T, class U, std::size_t N>
YK_POLYFILL_CXX14_CONSTEXPR void swap_impl(strategy_tag<strategy::array>, T (&a)[N], U (&b)[N]) noexcept(nothrow_swappable<T&, U&>::value)
{
  for (std::size_t i = 0; i < N; ++i) {
    ranges_swap::swap_impl(strategy_tag<strategy_of<T&, U&>::value>{}, a[i], b[i]);
  }
}

struct swap_fn {
  template<class T, class U, typename std::enable_if<strategy_of<T, U>::value != strategy::none, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR void operator()(T&& t, U&& u) const noexcept(nothrow_swappable<T, U>::value)
  {
    // qualified, so the call does no ADL over the argument types (see adl_swappable)
    ranges_swap::swap_impl(strategy_tag<strategy_of<T, U>::value>{}, std::forward<T>(t), std::forward<U>(u));
  }
};

}  // namespace ranges_swap

}  // namespace detail

namespace ranges {

YK_POLYFILL_INLINE constexpr detail::ranges_swap::swap_fn swap{};

}  // namespace ranges

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_CONCEPTS_HPP
