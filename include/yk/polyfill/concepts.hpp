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

// One specialization per strategy; each apply states its own noexcept, which callers query
// through noexcept(swap_impl_for<T, U>::apply(...)).
template<strategy S>
struct swap_impl;

template<class T, class U>
using swap_impl_for = swap_impl<strategy_of<T, U>::value>;

template<>
struct swap_impl<strategy::adl> {
  template<class T, class U>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T&& t, U&& u) noexcept(noexcept(swap(std::declval<T>(), std::declval<U>())))
  {
    (void)swap(std::forward<T>(t), std::forward<U>(u));
  }
};

template<>
struct swap_impl<strategy::exchange> {
  template<class T>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T& a, T& b) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value)
  {
    T tmp(std::move(a));
    a = std::move(b);
    b = std::move(tmp);
  }
};

template<>
struct swap_impl<strategy::array> {
  template<class T, class U, std::size_t N>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T (&a)[N], U (&b)[N]) noexcept(noexcept(swap_impl_for<T&, U&>::apply(std::declval<T&>(), std::declval<U&>())))
  {
    for (std::size_t i = 0; i < N; ++i) {
      swap_impl_for<T&, U&>::apply(a[i], b[i]);
    }
  }
};

struct swap_fn {
  template<class T, class U, typename std::enable_if<strategy_of<T, U>::value != strategy::none, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR void operator()(T&& t, U&& u) const noexcept(noexcept(swap_impl_for<T, U>::apply(std::declval<T>(), std::declval<U>())))
  {
    swap_impl_for<T, U>::apply(std::forward<T>(t), std::forward<U>(u));
  }
};

}  // namespace ranges_swap

// loose_swap: swap as [swappable.requirements] specifies it (an unqualified swap with std::swap in
// scope), but constexpr before C++20 where possible. ranges::swap is used where it provably selects
// what that context selects: an ADL swap that beats its deleted swap(T&, T&) also beats std::swap,
// and a type built only from fundamental types has neither an overload found by ADL nor, as a
// specialization must depend on a user-defined type, an explicit specialization of std::swap
// (allowed before C++20). Any other type goes through the std::swap context, which also covers
// non-movable types swappable through an overload for a base class or a constrained template.
// Where that context is ill-formed but ranges::swap applies, e.g. when two swaps found by ADL tie,
// ranges::swap is used, so loose_swap accepts what either accepts.
namespace std_swap_context {

using std::swap;

// [basic.lookup.argdep]: such a T has no associated namespaces or classes.
template<class T>
struct is_built_from_fundamental_types : std::is_fundamental<T> {};

template<class T>
struct is_built_from_fundamental_types<T*> : is_built_from_fundamental_types<T> {};

template<class T, std::size_t N>
struct is_built_from_fundamental_types<T[N]> : is_built_from_fundamental_types<T> {};

template<class T, ranges_swap::strategy S = ranges_swap::strategy_of<T&, T&>::value>
struct uses_ranges_swap : bool_constant<S == ranges_swap::strategy::adl || is_built_from_fundamental_types<T>::value || !is_swappable<T>::value> {};

template<class T>
struct uses_ranges_swap<T, ranges_swap::strategy::none> : false_type {};

template<bool UseRangesSwap>
struct loose_swap_impl;

template<>
struct loose_swap_impl<true> {
  template<class T>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T& a, T& b) noexcept(
      noexcept(ranges_swap::swap_impl_for<T&, T&>::apply(std::declval<T&>(), std::declval<T&>()))
  )
  {
    ranges_swap::swap_impl_for<T&, T&>::apply(a, b);
  }
};

template<>
struct loose_swap_impl<false> {
  template<class T>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T& a, T& b) noexcept(noexcept(swap(std::declval<T&>(), std::declval<T&>())))
  {
    swap(a, b);
  }
};

template<class T>
using loose_swap_impl_for = loose_swap_impl<uses_ranges_swap<T>::value>;

}  // namespace std_swap_context

template<
    class T,
    typename std::enable_if<ranges_swap::strategy_of<T&, T&>::value != ranges_swap::strategy::none || is_swappable<T>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR void loose_swap(T& a, T& b) noexcept(
    noexcept(std_swap_context::loose_swap_impl_for<T>::apply(std::declval<T&>(), std::declval<T&>()))
)
{
  std_swap_context::loose_swap_impl_for<T>::apply(a, b);
}

}  // namespace detail

namespace ranges {

YK_POLYFILL_INLINE constexpr detail::ranges_swap::swap_fn swap{};

}  // namespace ranges

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_CONCEPTS_HPP
