#ifndef YK_POLYFILL_TUPLE_HPP
#define YK_POLYFILL_TUPLE_HPP

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

namespace yk {

namespace polyfill {

namespace apply_detail {

// injects `std::get`, since unqualified `get<I>(t)` call won't trigger ADL until C++20
using std::get;

template<class IndexSeq>
struct apply_impl;

template<std::size_t... Is>
struct apply_impl<index_sequence<Is...>> {
  template<class F, class Tuple>
  static constexpr auto apply(F&& f, Tuple&& t) noexcept(noexcept(polyfill::invoke(static_cast<F&&>(f), get<Is>(static_cast<Tuple&&>(t))...)))
      -> decltype(polyfill::invoke(static_cast<F&&>(f), get<Is>(static_cast<Tuple&&>(t))...))
  {
    return polyfill::invoke(static_cast<F&&>(f), get<Is>(static_cast<Tuple&&>(t))...);
  }
};

template<class F, class Tuple, class = void>
struct is_applicable_impl : false_type {};

template<class F, class Tuple>
struct is_applicable_impl<
    F, Tuple,
    void_t<decltype(apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::
                        apply(std::declval<F>(), std::declval<Tuple>()))>> : true_type {};

template<class F, class Tuple, class = void>
struct is_nothrow_applicable_impl : false_type {};

template<class F, class Tuple>
struct is_nothrow_applicable_impl<
    F, Tuple,
    void_t<decltype(apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::
                        apply(std::declval<F>(), std::declval<Tuple>()))>>
    : bool_constant<noexcept(
          apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::apply(std::declval<F>(), std::declval<Tuple>())
      )> {};

template<class F, class Tuple, class = void>
struct apply_result_impl {};

template<class F, class Tuple>
struct apply_result_impl<
    F, Tuple,
    void_t<decltype(apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::
                        apply(std::declval<F>(), std::declval<Tuple>()))>> {
  using type = decltype(apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::apply(
      std::declval<F>(), std::declval<Tuple>()
  ));
};

}  // namespace apply_detail

template<class F, class Tuple>
struct is_applicable : apply_detail::is_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct is_nothrow_applicable : apply_detail::is_nothrow_applicable_impl<F, Tuple> {};

template<class F, class Tuple>
struct apply_result : apply_detail::apply_result_impl<F, Tuple> {};

template<class F, class Tuple>
constexpr typename apply_result<F, Tuple>::type apply(F&& f, Tuple&& t) noexcept(is_nothrow_applicable<F, Tuple>::value)
{
  return apply_detail::apply_impl<make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>::apply(
      static_cast<F&&>(f), static_cast<Tuple&&>(t)
  );
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_TUPLE_HPP
