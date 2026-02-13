#ifndef YK_POLYFILL_VARIANT_HPP
#define YK_POLYFILL_VARIANT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/bits/trivial_base.hpp>
#include <yk/polyfill/extension/pack_indexing.hpp>
#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/utility.hpp>

#include <exception>
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
struct select_index<Size, typename std::enable_if<(Size < 255)>::type> {
  using type = std::uint8_t;
};

template<std::size_t Size>
struct select_index<Size, typename std::enable_if<(255 <= Size && Size < 65535)>::type> {
  using type = std::uint16_t;
};

template<std::size_t I, class T, class... Us>
struct find_index_impl {};

template<std::size_t I, class T, class U, class... Us>
struct find_index_impl<I, T, U, Us...>
    : std::conditional<std::is_same<T, U>::value, std::integral_constant<std::size_t, I>, find_index_impl<I + 1, U, Us...>>::type {};

template<class T, class... Us>
struct find_index : find_index_impl<0, T, Us...> {};

template<bool Found, class T, class... Us>
struct exactly_once_impl : bool_constant<Found> {};

template<class T, class U, class... Us>
struct exactly_once_impl<false, T, U, Us...> : exactly_once_impl<std::is_same<T, U>::value, Us...> {};

template<class T, class U, class... Us>
struct exactly_once_impl<true, T, U, Us...> : std::conditional<std::is_same<T, U>::value, false_type, exactly_once_impl<true, T, Us...>>::type {};

template<class T, class... Us>
struct exactly_once : exactly_once_impl<false, T, Us...> {};

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
  constexpr variadic_union(in_place_index_t<I>, Args&&... args) : rest(in_place_index_t<I - 1>{}, std::forward<Args>(args)...)
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
  constexpr variadic_union(in_place_index_t<I>, Args&&... args) : rest(in_place_index_t<I - 1>{}, std::forward<Args>(args)...)
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
  using type = typename invoke_result<Visitor, in_place_index_t<0>, typename raw_get_result<0, Storage&&>::type>::type;
};

template<std::size_t I, class Visitor, class Storage>
constexpr typename raw_visit_result<Visitor, Storage&&>::type do_raw_visit(Visitor&& vis, Storage&& storage)
{
  return invoke(std::forward<Visitor>(vis), in_place_index<I>, raw_get<I>(std::forward<Storage>(storage)));
}

template<class Visitor, class Storage>
using raw_visit_function_type = typename raw_visit_result<Visitor, Storage&&>::type(Visitor&&, Storage&&);

template<class Visitor, class Storage, class IndexSeq = make_index_sequence<remove_cvref<Storage>::type::size()>>
struct raw_visit_table {};

template<class Visitor, class Storage, std::size_t... Is>
struct raw_visit_table<Visitor, Storage, index_sequence<Is...>> {
  static constexpr raw_visit_function_type<Visitor, Storage>* value[sizeof...(Is)]{&do_raw_visit<Is, Visitor, Storage>...};
};

template<class Visitor, class Storage, std::size_t... Is>
constexpr raw_visit_function_type<Visitor, Storage>* raw_visit_table<Visitor, Storage, index_sequence<Is...>>::value[sizeof...(Is)];

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
  template<std::size_t I, class T>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, T& x) noexcept
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
      : storage(in_place_index_t<0>{}), index(0)
  {
  }

  template<std::size_t I, class... Args>
  constexpr explicit variant_storage(in_place_index_t<I> ipi, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
      : storage(ipi, std::forward<Args>(args)...), index(I)
  {
  }

  struct constructor {
    variant_storage* self;

    template<std::size_t I, class Arg>
    YK_POLYFILL_CXX14_CONSTEXPR void operator()(in_place_index_t<I> ipi, Arg&& arg) /* noexcept */
    {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
      std::construct_at(&self->storage, ipi, std::forward<Arg>(arg));
#else
      new (&self->storage) storage_type(ipi, std::forward<Arg>(arg));
#endif
    }
  };

  YK_POLYFILL_CXX14_CONSTEXPR void construct_from(variant_storage const& other) { other.raw_visit(constructor{this}); }
  YK_POLYFILL_CXX14_CONSTEXPR void construct_from(variant_storage&& other) { other.raw_visit(constructor{this}); }
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

template<class T>
struct one_element_array {
  T data[1];
};

template<std::size_t Index, class Target, class Source, class = void>
struct imaginary_function {};

template<std::size_t Index, class Target, class Source>
struct imaginary_function<Index, Target, Source, void_t<decltype(one_element_array<Target>{{std::declval<Source>()}})>> {
  integral_constant<std::size_t, Index> operator()(Target);
};

template<class T, class IndexSeq, class... Ts>
struct imaginary_function_set_impl {};

template<class T, std::size_t... Is, class... Ts>
struct imaginary_function_set_impl<T, index_sequence<Is...>, Ts...> : imaginary_function<Is, Ts, T>... {};

template<class T, class... Ts>
struct imaginary_function_set : imaginary_function_set_impl<T, index_sequence_for<Ts...>, Ts...> {};

template<class, class T, class... Ts>
struct is_invocation_to_imaginary_function_set_valid_impl : false_type {};

template<class T, class... Ts>
struct is_invocation_to_imaginary_function_set_valid_impl<void_t<typename invoke_result<imaginary_function_set<T, Ts...>, T>::type>, T, Ts...> : true_type {};

template<class T, class... Ts>
struct is_invocation_to_imaginary_function_set_valid : is_invocation_to_imaginary_function_set_valid_impl<void, T, Ts...> {};

template<class T, class... Ts>
struct select_alternative : invoke_result<imaginary_function_set<T, Ts...>, T>::type {};

template<class T>
struct is_in_place_type : false_type {};

template<class T>
struct is_in_place_type<in_place_type_t<T>> : true_type {};

template<class T>
struct is_in_place_index : false_type {};

template<std::size_t I>
struct is_in_place_index<in_place_index_t<I>> : true_type {};

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

class bad_variant_access : public std::exception {
public:
  char const* what() const noexcept override { return "bad variant access"; }
};

template<class... Ts>
class variant : private trivial_base_detail::select_base_for_special_member_functions<typename variant_detail::make_variant_base<Ts...>::type> {
  static_assert(sizeof...(Ts) > 0, "variant must be instantiated with at least one type template parameter");

private:
  using base_type = trivial_base_detail::select_base_for_special_member_functions<typename variant_detail::make_variant_base<Ts...>::type>;

public:
  template<
      class Head = typename extension::pack_indexing<0, Ts...>::type,
      typename std::enable_if<std::is_default_constructible<Head>::value, std::nullptr_t>::type = nullptr>
  constexpr variant() noexcept(std::is_nothrow_default_constructible<Head>::value) : base_type(in_place_index_t<0>{})
  {
  }

  template<
      class T, typename std::enable_if<!std::is_same<typename remove_cvref<T>::type, variant>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<variant_detail::is_invocation_to_imaginary_function_set_valid<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!variant_detail::is_in_place_type<T>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!variant_detail::is_in_place_index<T>::value, std::nullptr_t>::type = nullptr,
      std::size_t SelectedIndex = variant_detail::select_alternative<T, Ts...>::value,
      class SelectedType = typename extension::pack_indexing<SelectedIndex, Ts...>::type,
      typename std::enable_if<std::is_constructible<SelectedType, T>::value, std::nullptr_t>::type = nullptr>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible<SelectedType, T>::value) : base_type(in_place_index_t<SelectedIndex>{}, std::forward<T>(t))
  {
  }

  template<
      std::size_t I, class... Args, class SelectedType = typename extension::pack_indexing<I, Ts...>::type,
      typename std::enable_if<std::is_constructible<SelectedType, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit variant(in_place_index_t<I> ipi, Args&&... args) noexcept(std::is_nothrow_constructible<SelectedType, Args...>::value)
      : base_type(ipi, std::forward<Args>(args)...)
  {
  }

  constexpr std::size_t index() const noexcept { return base_type::index; }

  template<std::size_t I, class... Us>
  friend constexpr typename variant_alternative<I, variant<Us...>>::type& get(variant<Us...>& v);

  template<std::size_t I, class... Us>
  friend constexpr typename variant_alternative<I, variant<Us...>>::type const& get(variant<Us...> const& v);

  template<std::size_t I, class... Us>
  friend constexpr typename variant_alternative<I, variant<Us...>>::type&& get(variant<Us...>&& v);

  template<std::size_t I, class... Us>
  friend constexpr typename variant_alternative<I, variant<Us...>>::type const&& get(variant<Us...> const&& v);
};

template<std::size_t I, class... Ts>
constexpr typename variant_alternative<I, variant<Ts...>>::type& get(variant<Ts...>& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(v.storage);
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
constexpr typename variant_alternative<I, variant<Ts...>>::type const& get(variant<Ts...> const& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(v.storage);
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
constexpr typename variant_alternative<I, variant<Ts...>>::type&& get(variant<Ts...>&& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(std::move(v.storage));
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
constexpr typename variant_alternative<I, variant<Ts...>>::type const&& get(variant<Ts...> const&& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(std::move(v.storage));
  }
  throw bad_variant_access{};
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_VARIANT_HPP
