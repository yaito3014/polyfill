#ifndef YK_POLYFILL_VARIANT_HPP
#define YK_POLYFILL_VARIANT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/cond_trivial_smf.hpp>
#include <yk/polyfill/bits/core_traits.hpp>

#include <yk/polyfill/extension/pack_indexing.hpp>

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/memory.hpp>
#include <yk/polyfill/utility.hpp>

#include <exception>
#include <utility>

#include <cstddef>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace yk {

namespace polyfill {

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

template<std::size_t Size>
struct variant_npos_for {
  static constexpr typename select_index<Size>::type value = -1;
};

template<std::size_t I, class T, class... Us>
struct find_index_impl {};

template<std::size_t I, class T, class U, class... Us>
struct find_index_impl<I, T, U, Us...>
    : std::conditional<std::is_same<T, U>::value, std::integral_constant<std::size_t, I>, find_index_impl<I + 1, T, Us...>>::type {};

template<class T, class... Us>
struct find_index : find_index_impl<0, T, Us...> {};

template<bool Found, class T, class... Us>
struct exactly_once_impl : bool_constant<Found> {};

template<class T, class U, class... Us>
struct exactly_once_impl<false, T, U, Us...> : exactly_once_impl<std::is_same<T, U>::value, T, Us...> {};

template<class T, class U, class... Us>
struct exactly_once_impl<true, T, U, Us...> : std::conditional<std::is_same<T, U>::value, false_type, exactly_once_impl<true, T, Us...>>::type {};

template<class T, class... Us>
struct exactly_once : exactly_once_impl<false, T, Us...> {};

struct valueless_t {};

YK_POLYFILL_INLINE constexpr valueless_t valueless{};

template<class... Ts>
struct is_never_valueless : conjunction<std::is_trivially_copyable<Ts>...> {};

template<bool TriviallyDestructible, class... Ts>
struct variadic_union {
  static constexpr bool never_valueless = false;

  constexpr variadic_union(valueless_t) {}

  static constexpr std::size_t size() noexcept { return sizeof...(Ts); }
};

template<class... Ts>
struct make_variadic_union {
  using type = variadic_union<conjunction<std::is_trivially_destructible<Ts>...>::value, Ts...>;
};

template<class Head, class... Rest>
struct variadic_union<true, Head, Rest...> {
  static constexpr bool never_valueless = false;  // FIXME

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
  static constexpr bool never_valueless = false;  // FIXME

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

template<bool NeverValueless>
constexpr std::size_t bias(std::size_t) noexcept;

template<>
constexpr std::size_t bias<true>(std::size_t x) noexcept
{
  return x;
}

template<>
constexpr std::size_t bias<false>(std::size_t x) noexcept
{
  return x + 1;
}

template<bool NeverValueless>
constexpr std::size_t unbias(std::size_t) noexcept;

template<>
constexpr std::size_t unbias<true>(std::size_t x) noexcept
{
  return x;
}

template<>
constexpr std::size_t unbias<false>(std::size_t x) noexcept
{
  return x - 1;
}

template<std::size_t UnbiasedI, class Storage>
struct raw_get_result {};

template<std::size_t UnbiasedI, bool TriviallyDestructible, class... Ts>
struct raw_get_result<UnbiasedI, variadic_union<TriviallyDestructible, Ts...>&> {
  using type = typename extension::pack_indexing<UnbiasedI, Ts...>::type&;
};

template<std::size_t UnbiasedI, bool TriviallyDestructible, class... Ts>
struct raw_get_result<UnbiasedI, variadic_union<TriviallyDestructible, Ts...> const&> {
  using type = typename extension::pack_indexing<UnbiasedI, Ts...>::type const&;
};

template<std::size_t UnbiasedI, bool TriviallyDestructible, class... Ts>
struct raw_get_result<UnbiasedI, variadic_union<TriviallyDestructible, Ts...>&&> {
  using type = typename extension::pack_indexing<UnbiasedI, Ts...>::type&&;
};

template<std::size_t UnbiasedI, bool TriviallyDestructible, class... Ts>
struct raw_get_result<UnbiasedI, variadic_union<TriviallyDestructible, Ts...> const&&> {
  using type = typename extension::pack_indexing<UnbiasedI, Ts...>::type const&&;
};

template<std::size_t UnbiasedI>
struct raw_get_impl {
  template<class StorageFWD>
  static constexpr typename raw_get_result<UnbiasedI, StorageFWD&&>::type apply(StorageFWD&& storage) noexcept
  {
    return raw_get_impl<UnbiasedI - 1>::apply(std::forward<StorageFWD>(storage).rest);
  }
};

template<>
struct raw_get_impl<0> {
  template<class StorageFWD>
  static constexpr typename raw_get_result<0, StorageFWD&&>::type apply(StorageFWD&& storage) noexcept
  {
    return std::forward<StorageFWD>(storage).head;
  }
};

template<std::size_t UnbiasedI, class StorageFWD>
constexpr typename raw_get_result<UnbiasedI, StorageFWD&&>::type raw_get(StorageFWD&& storage) noexcept
{
  return raw_get_impl<UnbiasedI>::apply(std::forward<StorageFWD>(storage));
}

template<
    class VisitorFWD, class StorageFWD, class Storage = typename remove_cvref<StorageFWD>::type,
    class BiasedIndexSeq = make_index_sequence<bias<Storage::never_valueless>(Storage::size())>>
struct raw_visit_noexcept {};

template<class VisitorFWD, class StorageFWD, class Storage, std::size_t... BiasedIs>
struct raw_visit_noexcept<VisitorFWD, StorageFWD, Storage, index_sequence<BiasedIs...>>
    : conjunction<is_nothrow_invocable<VisitorFWD, in_place_index_t<BiasedIs>, StorageFWD>...> {};

template<class VisitorFWD, class StorageFWD>
struct raw_visit_result {
  using type = typename invoke_result<VisitorFWD, in_place_index_t<0>, typename raw_get_result<0, StorageFWD&&>::type>::type;
};

template<bool NeverValueless, std::size_t BiasedI>
struct do_raw_visit_impl {
  static constexpr std::size_t UnbiasedI = unbias<NeverValueless>(BiasedI);

  template<class VisitorFWD, class StorageFWD>
  static constexpr typename raw_visit_result<VisitorFWD, StorageFWD&&>::type apply(VisitorFWD&& vis, StorageFWD&& storage) noexcept(
      raw_visit_noexcept<VisitorFWD, StorageFWD>::value
  )
  {
    return invoke(std::forward<VisitorFWD>(vis), in_place_index_t<UnbiasedI>{}, raw_get<UnbiasedI>(std::forward<StorageFWD>(storage)));
  }
};

// When `NeverValueless` is false and BiasedI is 0, it means `raw_visit` is attempting to access valueless variant.
// Instead of calling `raw_get`, `do_raw_visit` passes `storage` itself.
template<>
struct do_raw_visit_impl<false, 0> {
  template<class VisitorFWD, class StorageFWD>
  static constexpr typename raw_visit_result<VisitorFWD, StorageFWD&&>::type apply(VisitorFWD&& vis, StorageFWD&& storage) noexcept(
      raw_visit_noexcept<VisitorFWD, StorageFWD>::value
  )
  {
    return invoke(std::forward<VisitorFWD>(vis), in_place_index_t<variant_npos>{}, std::forward<StorageFWD>(storage));
  }
};

template<std::size_t BiasedI, class VisitorFWD, class StorageFWD>
constexpr typename raw_visit_result<VisitorFWD, StorageFWD&&>::type do_raw_visit(VisitorFWD&& vis, StorageFWD&& storage) noexcept(
    raw_visit_noexcept<VisitorFWD, StorageFWD>::value
)
{
  using Storage = typename remove_cvref<StorageFWD>::type;
  return do_raw_visit_impl<Storage::never_valueless, BiasedI>::apply(std::forward<VisitorFWD>(vis), std::forward<StorageFWD>(storage));
}

template<class VisitorFWD, class StorageFWD>
using raw_visit_function_type =
    typename raw_visit_result<VisitorFWD, StorageFWD&&>::type(VisitorFWD&&, StorageFWD&&) noexcept(raw_visit_noexcept<VisitorFWD, StorageFWD>::value);

template<
    class VisitorFWD, class StorageFWD, class Storage = typename remove_cvref<StorageFWD>::type,
    class BiasedIndexSeq = make_index_sequence<bias<Storage::never_valueless>(Storage::size())>>
struct raw_visit_table {};

template<class VisitorFWD, class StorageFWD, class Storage, std::size_t... BiasedIs>
struct raw_visit_table<VisitorFWD, StorageFWD, Storage, index_sequence<BiasedIs...>> {
  static constexpr raw_visit_function_type<VisitorFWD, StorageFWD>* value[sizeof...(BiasedIs)]{&do_raw_visit<BiasedIs, VisitorFWD, StorageFWD>...};
};

template<class VisitorFWD, class StorageFWD, class Storage, std::size_t... BiasedIs>
constexpr raw_visit_function_type<VisitorFWD, StorageFWD>*
    raw_visit_table<VisitorFWD, StorageFWD, Storage, index_sequence<BiasedIs...>>::value[sizeof...(BiasedIs)];

struct raw_visit_dispatch {
  template<class VisitorFWD, class StorageFWD>
  static constexpr typename raw_visit_result<VisitorFWD, StorageFWD&&>::type apply(VisitorFWD&& vis, StorageFWD&& storage, std::size_t biased_i) noexcept(
      raw_visit_noexcept<VisitorFWD, StorageFWD>::value
  )
  {
    return invoke(raw_visit_table<VisitorFWD, StorageFWD>::value[biased_i], std::forward<VisitorFWD>(vis), std::forward<StorageFWD>(storage));
  }
};

template<std::size_t I, class T, bool TriviallyDestructible = std::is_trivially_destructible<T>::value>
struct variant_destroyer_impl;

template<std::size_t I, class T>
struct variant_destroyer_impl<I, T, true> {
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T&&) {}
};

template<std::size_t I, class T>
struct variant_destroyer_impl<I, T, false> {
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(T&& x) { x.~T(); }
};

template<class T>
struct variant_destroyer_impl<variant_npos, T, true> {
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T&&) {}
};

template<class T>
struct variant_destroyer_impl<variant_npos, T, false> {
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(T&&) {}
};

struct variant_destroyer {
  template<std::size_t I, class T>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, T&& x) noexcept  // FIXME
  {
    variant_destroyer_impl<I, T>::apply(x);
  }
};

template<class... Ts>
struct variant_storage;

template<std::size_t I, class... Ts>
struct valueless_constructor_impl {
  template<class U>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>* self, U&& u) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, U>::value
  )
  {
    self->template construct_on_valueless<I>(std::forward<U>(u));
  }
};

template<class... Ts>
struct valueless_constructor_impl<variant_npos, Ts...> {
  template<class U>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>*, U&&) noexcept
  {
    // no-op
  }
};

template<class... Ts>
struct valueless_constructor {
  variant_storage<Ts...>* self;

  template<std::size_t I, class U>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, U&& u) noexcept
  {
    valueless_constructor_impl<I, Ts...>::apply(self, std::forward<U>(u));
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
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), storage, bias<storage_type::never_valueless>(index));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, storage_type const&>::type raw_visit(Visitor&& vis) const&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), storage, bias<storage_type::never_valueless>(index));
  }

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, storage_type&&>::type raw_visit(Visitor&& vis) &&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(storage), bias<storage_type::never_valueless>(index));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, storage_type const&&>::type raw_visit(Visitor&& vis) const&&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(storage), bias<storage_type::never_valueless>(index));
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

  constexpr bool valueless_by_exception() const noexcept { return index == variant_npos_for<sizeof...(Ts)>::value; }

  template<std::size_t I, class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_on_valueless(Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
  {
    polyfill::construct_at(&storage, in_place_index_t<I>{}, std::forward<Args>(args)...);
    index = I;
  }

  YK_POLYFILL_CXX17_CONSTEXPR void reset() noexcept(conjunction<std::is_nothrow_destructible<Ts>...>::value)
  {
    raw_visit(variant_destroyer{});
    index = variant_npos_for<sizeof...(Ts)>::value;
  }

  template<std::size_t I, class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void emplace(Args&&... args)  // TODO: add noexcept
  {
    if (!valueless_by_exception()) reset();
    construct_on_valueless<I>(std::forward<Args>(args)...);
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_construct(variant_storage const& other) noexcept(conjunction<std::is_nothrow_copy_constructible<Ts>...>::value)
  {
    other.raw_visit(valueless_constructor<Ts...>{this});
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_construct(variant_storage&& other) noexcept(conjunction<std::is_nothrow_move_constructible<Ts>...>::value)
  {
    std::move(other).raw_visit(valueless_constructor<Ts...>{this});
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
class variant : private cond_trivial_smf<typename variant_detail::make_variant_base<Ts...>::type, Ts...> {
  static_assert(sizeof...(Ts) > 0, "variant must be instantiated with at least one type template parameter");

private:
  using base_type = cond_trivial_smf<typename variant_detail::make_variant_base<Ts...>::type, Ts...>;

public:
  using base_type::emplace;
  using base_type::valueless_by_exception;

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

  constexpr std::size_t index() const noexcept { return valueless_by_exception() ? variant_npos : base_type::index; }

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
