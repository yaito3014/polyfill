#ifndef YK_ZZ_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP
#define YK_ZZ_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace extension {

namespace detail {

template<class From, class To, class = void>
struct narrowing_check : false_type {};

template<class From, class To>
struct narrowing_check<From, To, void_t<decltype(typename type_identity<To[]>::type{std::declval<From>()})>> : true_type {};

// `is_convertible_without_narrowing` uses `std::is_constructible` internally, since for object types (or reference to them),
// reference-stripped `To` must be at least constructible from `From` for conversion to occur. For non-object types, it respects `std::is_convertible`.

// Primary template: respect `std::is_convertible`
template<class From, class To, class = void>
struct is_convertible_without_narrowing_impl : true_type {};

// When reference-stripped `To` IS constructible from `From`, check whether narrowing occurs
template<class From, class To>
struct is_convertible_without_narrowing_impl<
    From, To, typename std::enable_if<std::is_constructible<typename std::remove_reference<To>::type, From>::value>::type>
    : narrowing_check<From, typename std::remove_reference<To>::type> {};

}  // namespace detail

template<class From, class To>
struct is_convertible_without_narrowing : conjunction<std::is_convertible<From, To>, detail::is_convertible_without_narrowing_impl<From, To>> {};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_EXTENSION_IS_CONVERTIBLE_WITHOUT_NARROWING_HPP
