#ifndef YK_POLYFILL_VARIANT_HPP
#define YK_POLYFILL_VARIANT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/extension/pack_indexing.hpp>
#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

#include <utility>

#include <cstddef>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace yk {

namespace polyfill {

namespace variant_detail {

template<std::size_t Size, class = void>
struct select_index {
  using type = std::size_t;
};

template<std::size_t Size>
struct select_index<Size, typename std::enable_if<(0 <= Size && Size <= 256)>::type> {
  using type = std::uint8_t;
};

template<std::size_t Size>
struct select_index<Size, typename std::enable_if<(256 < Size && Size <= 65536)>::type> {
  using type = std::uint16_t;
};

struct valueless_t {};

YK_POLYFILL_INLINE constexpr valueless_t valueless{};

template<bool TriviallyDestructible, class... Ts>
struct variadic_union {
  constexpr variadic_union(valueless_t) {}

  static constexpr std::size_t size() noexcept { return sizeof...(Ts); }
};

template<class... Ts>
struct make_variadic_union {
  using type = variadic_union<conjunction<std::is_trivially_destructible<Ts>...>::value, Ts...>;
};

template<class Head, class... Rest>
struct variadic_union<true, Head, Rest...> {
  using head_type = Head;
  using rest_type = typename make_variadic_union<Rest...>::type;

  union {
    head_type head;
    rest_type rest;
  };

  constexpr variadic_union(valueless_t vl) : rest(vl) {}

  template<class... Args>
  constexpr variadic_union(in_place_index_t<0>, Args&&... args) : head(std::forward<Args>(args)...)
  {
  }

  template<std::size_t I, class... Args>
  constexpr variadic_union(in_place_index_t<I>, Args&&... args) : rest(in_place_index<I - 1>, std::forward<Args>(args)...)
  {
  }

  static constexpr std::size_t size() noexcept { return 1 + sizeof...(Rest); }
};

template<class Head, class... Rest>
struct variadic_union<false, Head, Rest...> {
  using head_type = Head;
  using rest_type = typename make_variadic_union<Rest...>::type;

  union {
    head_type head;
    rest_type rest;
  };

  YK_POLYFILL_CXX20_CONSTEXPR ~variadic_union() noexcept {}

  constexpr variadic_union(valueless_t vl) : rest(vl) {}

  template<class... Args>
  constexpr variadic_union(in_place_index_t<0>, Args&&... args) : head(std::forward<Args>(args)...)
  {
  }

  template<std::size_t I, class... Args>
  constexpr variadic_union(in_place_index_t<I>, Args&&... args) : rest(in_place_index<I - 1>, std::forward<Args>(args)...)
  {
  }

  static constexpr std::size_t size() noexcept { return 1 + sizeof...(Rest); }
};

template<std::size_t I, class Storage>
struct raw_get_result {};

template<std::size_t I, bool TriviallyDestructible, class... Ts>
struct raw_get_result<I, variadic_union<TriviallyDestructible, Ts...>&> {
  using type = typename extension::pack_indexing<I, Ts...>::type&;
};

template<std::size_t I, bool TriviallyDestructible, class... Ts>
struct raw_get_result<I, variadic_union<TriviallyDestructible, Ts...> const&> {
  using type = typename extension::pack_indexing<I, Ts...>::type const&;
};

template<std::size_t I, bool TriviallyDestructible, class... Ts>
struct raw_get_result<I, variadic_union<TriviallyDestructible, Ts...>&&> {
  using type = typename extension::pack_indexing<I, Ts...>::type&&;
};

template<std::size_t I, bool TriviallyDestructible, class... Ts>
struct raw_get_result<I, variadic_union<TriviallyDestructible, Ts...> const&&> {
  using type = typename extension::pack_indexing<I, Ts...>::type const&&;
};

template<std::size_t I>
struct raw_get_impl {
  template<class Storage>
  static constexpr typename raw_get_result<I, Storage&&>::type apply(Storage&& storage) noexcept
  {
    return raw_get_impl<I - 1>::apply(std::forward<Storage>(storage).rest);
  }
};

template<>
struct raw_get_impl<0> {
  template<class Storage>
  static constexpr typename raw_get_result<0, Storage&&>::type apply(Storage&& storage) noexcept
  {
    return std::forward<Storage>(storage).head;
  }
};

template<std::size_t I, class Storage>
constexpr typename raw_get_result<I, Storage&&>::type raw_get(Storage&& storage) noexcept
{
  return raw_get_impl<I>::apply(std::forward<Storage>(storage));
}

template<class Visitor, class Storage>
struct raw_visit_result {
  using type = typename invoke_result<Visitor, typename raw_get_result<0, Storage&&>::type>::type;
};

template<std::size_t I, class Visitor, class Storage>
constexpr typename raw_visit_result<Visitor, Storage&&>::type do_raw_visit(Visitor&& vis, Storage&& storage)
{
  return invoke(std::forward<Visitor>(vis), raw_get<I>(std::forward<Storage>(storage)));
}

template<class Visitor, class Storage>
using raw_visit_function_type = typename raw_visit_result<Visitor, Storage&&>::type(Visitor&&, Storage&&);

template<class Visitor, class Storage, class IndexSeq = make_index_sequence<remove_cvref<Storage>::type::size()>>
struct raw_visit_table {};

template<class Visitor, class Storage, std::size_t... Is>
struct raw_visit_table<Visitor, Storage, index_sequence<Is...>> {
  static constexpr raw_visit_function_type<Visitor, Storage>* value[sizeof...(Is)]{&do_raw_visit<Is, Visitor, Storage>...};
};

struct raw_visit_dispatch {
  template<class Visitor, class Storage>
  static constexpr typename raw_visit_result<Visitor, Storage&&>::type apply(Visitor&& vis, Storage&& storage, std::size_t index) noexcept(
      is_nothrow_invocable<raw_visit_function_type<Visitor, Storage>*, Visitor, Storage, std::size_t>::value
  )
  {
    return invoke(raw_visit_table<Visitor, Storage>::value[index], std::forward<Visitor>(vis), std::forward<Storage>(storage));
  }
};

template<class T, bool TriviallyDestructible = std::is_trivially_destructible<T>::value>
struct variant_destroyer_impl;

template<class T>
struct variant_destroyer_impl<T, true> {
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T&) {}
};

template<class T>
struct variant_destroyer_impl<T, false> {
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(T& x) { x.~T(); }
};

struct variant_destroyer {
  template<class T>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(T& x) noexcept
  {
    variant_destroyer_impl<T>::apply(x);
  }
};

template<class... Ts>
struct variant_storage {
  using storage_type = typename variant_detail::make_variadic_union<Ts...>::type;
  using index_type = typename variant_detail::select_index<sizeof...(Ts)>::type;

  storage_type storage;
  index_type index;

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, storage_type&>::type raw_visit(Visitor&& vis) &
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), storage, index);
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, storage_type const&>::type raw_visit(Visitor&& vis) const&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), storage, index);
  }

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, storage_type&&>::type raw_visit(Visitor&& vis) &&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(storage), index);
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, storage_type const&&>::type raw_visit(Visitor&& vis) const&&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(storage), index);
  }

  constexpr variant_storage() noexcept(std::is_nothrow_default_constructible<typename extension::pack_indexing<0, Ts...>::type>::value)
      : storage(in_place_index<0>), index(0)
  {
  }

  template<std::size_t I, class... Args>
  constexpr explicit variant_storage(in_place_index_t<I> ipi, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
      : storage(ipi, std::forward<Args>(args)...), index(I)
  {
  }
};

template<bool TriviallyDestructible, class... Ts>
struct variant_base;

template<class... Ts>
struct variant_base<true, Ts...> : variant_storage<Ts...> {
  using variant_storage<Ts...>::variant_storage;
};

template<class... Ts>
struct variant_base<false, Ts...> : variant_storage<Ts...> {
  using variant_storage<Ts...>::variant_storage;

  YK_POLYFILL_CXX20_CONSTEXPR ~variant_base() { this->raw_visit(variant_destroyer{}); }
};

template<class... Ts>
struct make_variant_base {
  using type = variant_base<conjunction<std::is_trivially_destructible<Ts>...>::value, Ts...>;
};

}  // namespace variant_detail

template<class... Ts>
class variant;

template<class T>
struct variant_size;

template<class T>
struct variant_size<T const> : variant_size<T> {};

template<class... Ts>
struct variant_size<variant<Ts...>> : integral_constant<std::size_t, sizeof...(Ts)> {};

template<std::size_t I, class T>
struct variant_alternative;

template<std::size_t I, class T>
struct variant_alternative<I, T const> : variant_alternative<I, T> {};

template<std::size_t I, class... Ts>
struct variant_alternative<I, variant<Ts...>> : extension::pack_indexing<I, Ts...> {};

YK_POLYFILL_INLINE constexpr std::size_t variant_npos = -1;

struct monostate {};

template<class... Ts>
class variant : private variant_detail::make_variant_base<Ts...>::type {
private:
  using base_type = typename variant_detail::make_variant_base<Ts...>::type;

public:
  template<
      class Head = typename extension::pack_indexing<0, Ts...>::type,
      typename std::enable_if<std::is_default_constructible<Head>::value, std::nullptr_t>::type = nullptr>
  constexpr variant() noexcept(std::is_nothrow_default_constructible<Head>::value) : base_type(in_place_index<0>)
  {
  }

  template<
      std::size_t I, class... Args,
      typename std::enable_if<std::is_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit variant(in_place_index_t<I> ipi, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
      : base_type(ipi, std::forward<Args>(args)...)
  {
  }

  constexpr std::size_t index() const noexcept { return base_type::index; }
};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_VARIANT_HPP
