#ifndef YK_POLYFILL_TUPLE_HPP
#define YK_POLYFILL_TUPLE_HPP

#include <yk/polyfill/bits/apply_detail.hpp>

#include <yk/polyfill/type_traits.hpp>

namespace yk {

namespace polyfill {

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
