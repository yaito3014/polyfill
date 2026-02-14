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

  // When at least one type is non-trivially destructible, union cannot be
  // non-trivially destructible. So the destructor is manually defined here
  // and destruction of contained value is handled by `variant_base`.
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

template<std::size_t I, class Union>
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
  template<class UnionT>
  static constexpr typename raw_get_result<I, UnionT&&>::type apply(UnionT&& vunion) noexcept
  {
    return raw_get_impl<I - 1>::apply(std::forward<UnionT>(vunion).rest);
  }
};

template<>
struct raw_get_impl<0> {
  template<class UnionT>
  static constexpr typename raw_get_result<0, UnionT&&>::type apply(UnionT&& vunion) noexcept
  {
    return std::forward<UnionT>(vunion).head;
  }
};

template<std::size_t I, class UnionT>
constexpr typename raw_get_result<I, UnionT&&>::type raw_get(UnionT&& vunion) noexcept
{
  return raw_get_impl<I>::apply(std::forward<UnionT>(vunion));
}

template<
    class VisitorT, class UnionT, class Union = typename remove_cvref<UnionT>::type,
    class BiasedIndexSeq = make_index_sequence<bias<Union::never_valueless>(Union::size())>>
struct raw_visit_noexcept {};

template<class VisitorT, class UnionT, class Union, std::size_t... BiasedIs>
struct raw_visit_noexcept<VisitorT, UnionT, Union, index_sequence<BiasedIs...>>
    : conjunction<is_nothrow_invocable<VisitorT, in_place_index_t<BiasedIs>, UnionT>...> {};

template<class VisitorT, class UnionT>
struct raw_visit_result {
  using type = typename invoke_result<VisitorT, in_place_index_t<0>, typename raw_get_result<0, UnionT&&>::type>::type;
};

template<bool NeverValueless, std::size_t BiasedI>
struct do_raw_visit_impl {
  static constexpr std::size_t I = unbias<NeverValueless>(BiasedI);

  template<class VisitorT, class UnionT>
  static constexpr typename raw_visit_result<VisitorT, UnionT&&>::type apply(VisitorT&& vis, UnionT&& vunion) noexcept(
      raw_visit_noexcept<VisitorT, UnionT>::value
  )
  {
    return invoke(std::forward<VisitorT>(vis), in_place_index_t<I>{}, raw_get<I>(std::forward<UnionT>(vunion)));
  }
};

// When `NeverValueless` is false and BiasedI is 0, it means `raw_visit` is attempting to access valueless variant.
// Instead of calling `raw_get`, `do_raw_visit` passes `vunion` itself.
template<>
struct do_raw_visit_impl<false, 0> {
  template<class VisitorT, class UnionT>
  static constexpr typename raw_visit_result<VisitorT, UnionT&&>::type apply(VisitorT&& vis, UnionT&& vunion) noexcept(
      raw_visit_noexcept<VisitorT, UnionT>::value
  )
  {
    return invoke(std::forward<VisitorT>(vis), in_place_index_t<variant_npos>{}, std::forward<UnionT>(vunion));
  }
};

template<std::size_t BiasedI, class VisitorT, class UnionT>
constexpr typename raw_visit_result<VisitorT, UnionT&&>::type do_raw_visit(VisitorT&& vis, UnionT&& vunion) noexcept(
    raw_visit_noexcept<VisitorT, UnionT>::value
)
{
  using Union = typename remove_cvref<UnionT>::type;
  return do_raw_visit_impl<Union::never_valueless, BiasedI>::apply(std::forward<VisitorT>(vis), std::forward<UnionT>(vunion));
}

template<class VisitorT, class UnionT>
using raw_visit_function_type =
    typename raw_visit_result<VisitorT, UnionT&&>::type(VisitorT&&, UnionT&&) YK_POLYFILL_CXX17_NOEXCEPT(raw_visit_noexcept<VisitorT, UnionT>::value);

template<
    class VisitorT, class UnionT, class Union = typename remove_cvref<UnionT>::type,
    class BiasedIndexSeq = make_index_sequence<bias<Union::never_valueless>(Union::size())>>
struct raw_visit_table {};

template<class VisitorT, class UnionT, class Union, std::size_t... BiasedIs>
struct raw_visit_table<VisitorT, UnionT, Union, index_sequence<BiasedIs...>> {
  static constexpr raw_visit_function_type<VisitorT, UnionT>* value[sizeof...(BiasedIs)]{&do_raw_visit<BiasedIs, VisitorT, UnionT>...};
};

template<class VisitorT, class UnionT, class Union, std::size_t... BiasedIs>
constexpr raw_visit_function_type<VisitorT, UnionT>* raw_visit_table<VisitorT, UnionT, Union, index_sequence<BiasedIs...>>::value[sizeof...(BiasedIs)];

struct raw_visit_dispatch {
  template<class VisitorT, class UnionT>
  static constexpr typename raw_visit_result<VisitorT, UnionT&&>::type apply(VisitorT&& vis, UnionT&& vunion, std::size_t biased_i) noexcept(
      raw_visit_noexcept<VisitorT, UnionT>::value
  )
  {
    return invoke(raw_visit_table<VisitorT, UnionT>::value[biased_i], std::forward<VisitorT>(vis), std::forward<UnionT>(vunion));
  }
};

template<class... Ts>
struct variant_storage;

template<std::size_t I, class Operation>
struct no_op_wrapper {
  template<class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(Args&&... args)  // TODO: add noexcept
  {
    Operation::template apply</* Valid */ I>(std::forward<Args>(args)...);
  }
};

template<class Operation>
struct no_op_wrapper<variant_npos, Operation> {
  template<class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(Args&&...) noexcept
  {
    // no-op
  }
};

template<bool TriviallyDestructible>
struct destroy_operation;

template<>
struct destroy_operation</*TriviallyDestructible = */ true> {
  template<std::size_t /* ValidI */, class ContainedT>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(ContainedT&& value) noexcept
  {
    // no-op
  }
};

template<>
struct destroy_operation</*TriviallyDestructible = */ false> {
  template<std::size_t /* ValidI */, class ContainedT>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(ContainedT&& value) noexcept
  {
    using T = typename remove_cvref<ContainedT>::type;
    value.~T();
  }
};

struct destroy_visitor {
  template<std::size_t I, class ContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, ContainedT&& value) noexcept
  {
    using T = typename remove_cvref<ContainedT>::type;
    no_op_wrapper<I, destroy_operation<std::is_trivially_destructible<T>::value>>::apply(std::forward<ContainedT>(value));
  }
};

template<bool TriviallyDestructible>
struct reset_operation;

template<>
struct reset_operation</*TriviallyDestructible = */ true> {
  template<std::size_t /* ValidI */, class ContainedT, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, ContainedT&&) noexcept
  {
    storage.index = variant_npos_for<sizeof...(Ts)>::value;
  }
};

template<>
struct reset_operation</*TriviallyDestructible = */ false> {
  template<std::size_t /* ValidI */, class ContainedT, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, ContainedT&& value) noexcept
  {
    using T = typename remove_cvref<ContainedT>::type;
    value.~T();
    storage.index = variant_npos_for<sizeof...(Ts)>::value;
  }
};

template<class... Ts>
struct reset_visitor {
  variant_storage<Ts...>& storage;

  template<std::size_t I, class ContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, ContainedT&& value) noexcept
  {
    using T = typename remove_cvref<ContainedT>::type;
    no_op_wrapper<I, reset_operation<std::is_trivially_destructible<T>::value>>::apply(storage, std::forward<ContainedT>(value));
  }
};

struct construct_on_valueless_operation {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args)  // TODO: add noexcept
  {
    polyfill::construct_at(&storage.vunion, in_place_index_t<ValidI>{}, std::forward<Args>(args)...);
    storage.index = ValidI;
  }
};

template<class... Ts>
struct construct_on_valueless_visitor {
  variant_storage<Ts...>& storage;

  template<std::size_t J, class OtherContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, OtherContainedT&& other_value)  // TODO: add noexcept
  {
    no_op_wrapper<J, construct_on_valueless_operation>::apply(storage, std::forward<OtherContainedT>(other_value));
  }
};

template<bool IsCopySaferThanMove>
struct emplace_or_move_assign_temporary_operation;

template<>
struct emplace_or_move_assign_temporary_operation</*IsCopySaferThanMove = */ true> {
  template<std::size_t ValidJ, class RhsContained, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained const& rhs_value)  // TODO: add noexcept
  {
    lhs.template emplace<ValidJ>(rhs_value);
  }
};

template<>
struct emplace_or_move_assign_temporary_operation</*IsCopySaferThanMove = */ false> {
  template<std::size_t ValidJ, class RhsContained, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained const& rhs_value)  // TODO: add noexcept
  {
    RhsContained temporary(rhs_value);
    lhs.template emplace<ValidJ>(std::move(temporary));
  }
};

struct copy_assign_operation {
  template<std::size_t ValidJ, class RhsContained, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained const& rhs_value)  // TODO: add noexcept
  {
    if (lhs.index == ValidJ) {
      raw_get<ValidJ>(lhs.vunion) = rhs_value;
    } else {
      using T_j = typename extension::pack_indexing<ValidJ, Ts...>::type;
      constexpr bool IsCopySaferThanMove = disjunction<std::is_nothrow_copy_constructible<T_j>, negation<std::is_nothrow_move_constructible<T_j>>>::value;
      emplace_or_move_assign_temporary_operation<IsCopySaferThanMove>::template apply<ValidJ>(lhs, rhs_value);
    }
  }
};

template<class... Ts>
struct copy_assign_visitor {
  variant_storage<Ts...>& lhs;

  template<std::size_t J, class RhsContained>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, RhsContained const& rhs_value)  // TODO: add noexcept
  {
    no_op_wrapper<J, copy_assign_operation>::apply(lhs, rhs_value);
  }
};

struct move_assign_operation {
  template<std::size_t ValidJ, class RhsContained, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained&& rhs_value)  // TODO: add noexcept
  {
    if (lhs.index == ValidJ) {
      raw_get<ValidJ>(lhs.vunion) = std::move(rhs_value);
    } else {
      lhs.template emplace<ValidJ>(std::move(rhs_value));
    }
  }
};

template<class... Ts>
struct move_assign_visitor {
  variant_storage<Ts...>& lhs;

  template<std::size_t J, class RhsContained>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, RhsContained&& rhs_value)  // TODO: add noexcept
  {
    no_op_wrapper<J, move_assign_operation>::apply(lhs, rhs_value);
  }
};

template<class... Ts>
struct variant_storage {
  using union_type = typename variant_detail::make_variadic_union<Ts...>::type;
  using index_type = typename variant_detail::select_index<sizeof...(Ts)>::type;

  union_type vunion;
  index_type index;

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, union_type&>::type raw_visit(Visitor&& vis) &
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), vunion, bias<union_type::never_valueless>(index));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, union_type const&>::type raw_visit(Visitor&& vis) const&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), vunion, bias<union_type::never_valueless>(index));
  }

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, union_type&&>::type raw_visit(Visitor&& vis) &&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(vunion), bias<union_type::never_valueless>(index));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, union_type const&&>::type raw_visit(Visitor&& vis) const&&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(vunion), bias<union_type::never_valueless>(index));
  }

  constexpr variant_storage() noexcept(std::is_nothrow_default_constructible<typename extension::pack_indexing<0, Ts...>::type>::value)
      : vunion(in_place_index_t<0>{}), index(0)
  {
  }

  template<std::size_t I, class... Args>
  constexpr explicit variant_storage(in_place_index_t<I> ipi, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
      : vunion(ipi, std::forward<Args>(args)...), index(I)
  {
  }

  constexpr bool valueless_by_exception() const noexcept { return index == variant_npos_for<sizeof...(Ts)>::value; }

  // calls `visit` to destroy contained value and *DO NOT* set index
  YK_POLYFILL_CXX20_CONSTEXPR void dynamic_destroy() noexcept { raw_visit(destroy_visitor{}); }

  // calls `visit` to destroy contained value and *DO* set index
  YK_POLYFILL_CXX20_CONSTEXPR void dynamic_reset() noexcept { raw_visit(reset_visitor<Ts...>{*this}); }

  template<std::size_t ValidI, class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR typename extension::pack_indexing<ValidI, Ts...>::type& emplace(Args&&... args)  // TODO: add noexcept
  {
    dynamic_reset();
    construct_on_valueless_operation::apply<ValidI>(*this, std::forward<Args>(args)...);
    return raw_get<ValidI>(vunion);
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_construct(variant_storage const& other) noexcept(conjunction<std::is_nothrow_copy_constructible<Ts>...>::value)
  {
    other.raw_visit(construct_on_valueless_visitor<Ts...>{*this});
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_construct(variant_storage&& other) noexcept(conjunction<std::is_nothrow_move_constructible<Ts>...>::value)
  {
    std::move(other).raw_visit(construct_on_valueless_visitor<Ts...>{*this});
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_assign(variant_storage const& other) noexcept(
      conjunction<std::is_nothrow_copy_constructible<Ts>..., std::is_nothrow_copy_assignable<Ts>...>::value
  )
  {
    other.raw_visit(copy_assign_visitor<Ts...>{*this});
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_assign(variant_storage&& other)  // TODO: add noexcept
  {
    std::move(other).raw_visit(move_assign_visitor<Ts...>{*this});
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

  YK_POLYFILL_CXX20_CONSTEXPR ~variant_base() { this->raw_visit(destroy_visitor{}); }
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
      typename std::enable_if<!variant_detail::is_in_place_type<typename remove_cvref<T>::type>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!variant_detail::is_in_place_index<typename remove_cvref<T>::type>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<variant_detail::is_invocation_to_imaginary_function_set_valid<T, Ts...>::value, std::nullptr_t>::type = nullptr,
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
  friend YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Us...>>::type& get(variant<Us...>& v);

  template<std::size_t I, class... Us>
  friend YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Us...>>::type const& get(variant<Us...> const& v);

  template<std::size_t I, class... Us>
  friend YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Us...>>::type&& get(variant<Us...>&& v);

  template<std::size_t I, class... Us>
  friend YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Us...>>::type const&& get(variant<Us...> const&& v);
};

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type& get(variant<Ts...>& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(v.vunion);
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type const& get(variant<Ts...> const& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(v.vunion);
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type&& get(variant<Ts...>&& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(std::move(v.vunion));
  }
  throw bad_variant_access{};
}

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type const&& get(variant<Ts...> const&& v)
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v.index() == I) {
    return variant_detail::raw_get<I>(std::move(v.vunion));
  }
  throw bad_variant_access{};
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_VARIANT_HPP
