#ifndef YK_POLYFILL_EXTENSION_PACK_INDEXING_HPP
#define YK_POLYFILL_EXTENSION_PACK_INDEXING_HPP

#include <yk/polyfill/utility.hpp>

#include <cstddef>

namespace yk {

namespace polyfill {

namespace extension {

namespace pack_indexing_detail {

template<std::size_t I>
using make_void = void;

template<class T>
struct identity {};

template<class IndexSeq>
struct do_pack_indexing;

template<std::size_t... Is>
struct do_pack_indexing<index_sequence<Is...>> {
  template<class T>
  static T select(make_void<Is>*..., identity<T>*, ...);
};

}  // namespace pack_indexing_detail

template<std::size_t I, class... Ts>
struct pack_indexing {
  static_assert(I < sizeof...(Ts), "index must be less than size of parameter pack");
  using type = decltype(pack_indexing_detail::do_pack_indexing<make_index_sequence<I>>::select(static_cast<pack_indexing_detail::identity<Ts>*>(nullptr)...));
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_PACK_INDEXING_HPP
