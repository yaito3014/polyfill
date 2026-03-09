#ifndef YK_POLYFILL_VARIANT_HPP
#define YK_POLYFILL_VARIANT_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/cond_trivial_smf.hpp>
#include <yk/polyfill/bits/core_traits.hpp>

#include <yk/polyfill/extension/pack_indexing.hpp>

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/memory.hpp>
#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>

#include <exception>
#include <memory>
#include <utility>

#include <cstddef>
#include <cstdint>

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

#if __cplusplus >= 201402L

template<class T>
constexpr std::size_t variant_size_v = variant_size<T>::value;

template<std::size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

#endif

YK_POLYFILL_INLINE constexpr std::size_t variant_npos = -1;

struct monostate {};

class bad_variant_access : public std::exception {
public:
  char const* what() const noexcept override { return "bad variant access"; }
};

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type& get(variant<Ts...>&);

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type const& get(variant<Ts...> const&);

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type&& get(variant<Ts...>&&);

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR typename variant_alternative<I, variant<Ts...>>::type const&& get(variant<Ts...> const&&);

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
  static constexpr bool never_valueless = is_never_valueless<Head, Rest...>::value;

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
  static constexpr bool never_valueless = is_never_valueless<Head, Rest...>::value;

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
    class BiasedIndexSeq = make_index_sequence<variant_detail::bias<Union::never_valueless>(Union::size())>>
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
  static constexpr std::size_t I = variant_detail::unbias<NeverValueless>(BiasedI);

  template<class VisitorT, class UnionT>
  static constexpr typename raw_visit_result<VisitorT, UnionT&&>::type apply(VisitorT&& vis, UnionT&& vunion) noexcept(
      raw_visit_noexcept<VisitorT, UnionT>::value
  )
  {
    return polyfill::invoke(std::forward<VisitorT>(vis), in_place_index_t<I>{}, variant_detail::raw_get<I>(std::forward<UnionT>(vunion)));
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
    return polyfill::invoke(std::forward<VisitorT>(vis), in_place_index_t<variant_npos>{}, std::forward<UnionT>(vunion));
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
    class BiasedIndexSeq = make_index_sequence<variant_detail::bias<Union::never_valueless>(Union::size())>>
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
    return raw_visit_table<VisitorT, UnionT>::value[biased_i](std::forward<VisitorT>(vis), std::forward<UnionT>(vunion));
  }
};

template<class... Ts>
struct variant_storage;

template<std::size_t I, class Operation>
struct no_op_wrapper {
  template<class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(Args&&... args) noexcept(noexcept(Operation::template apply<I>(std::declval<Args>()...)))
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
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(ContainedT&& value) noexcept
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
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(variant_storage<Ts...>& storage, ContainedT&&) noexcept
  {
    storage.vindex = variant_npos_for<sizeof...(Ts)>::value;
  }
};

template<>
struct reset_operation</*TriviallyDestructible = */ false> {
  template<std::size_t /* ValidI */, class ContainedT, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, ContainedT&& value) noexcept
  {
    using T = typename remove_cvref<ContainedT>::type;
    value.~T();
    storage.vindex = variant_npos_for<sizeof...(Ts)>::value;
  }
};

template<class... Ts>
struct reset_visitor {
  variant_storage<Ts...>& storage;

  template<std::size_t I, class ContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, ContainedT&& value) noexcept
  {
    using Contained = typename remove_cvref<ContainedT>::type;
    no_op_wrapper<I, reset_operation<std::is_trivially_destructible<Contained>::value>>::apply(storage, std::forward<ContainedT>(value));
  }
};

enum class trivial_reconstruction { move_construct, copy_construct, move_assign, copy_assign };

template<class T>
constexpr trivial_reconstruction select_trivial_reconstruction()
{
  return std::is_trivially_move_constructible<T>::value   ? trivial_reconstruction::move_construct
         : std::is_trivially_copy_constructible<T>::value ? trivial_reconstruction::copy_construct
         : std::is_trivially_move_assignable<T>::value    ? trivial_reconstruction::move_assign
                                                          : trivial_reconstruction::copy_assign;
}

template<trivial_reconstruction>
struct reconstruct_operation;

template<>
struct reconstruct_operation<trivial_reconstruction::move_construct> {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<ValidI, Ts...>::type, Args...>::value
  )
  {
    using T_i = typename extension::pack_indexing<ValidI, Ts...>::type;
    T_i tmp(std::forward<Args>(args)...);  // may throw
    polyfill::construct_at(&storage.vunion, in_place_index_t<ValidI>{}, std::move(tmp));
    storage.vindex = ValidI;
  }
};

template<>
struct reconstruct_operation<trivial_reconstruction::copy_construct> {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<ValidI, Ts...>::type, Args...>::value
  )
  {
    using T_i = typename extension::pack_indexing<ValidI, Ts...>::type;
    T_i tmp(std::forward<Args>(args)...);  // may throw
    polyfill::construct_at(&storage.vunion, in_place_index_t<ValidI>{}, tmp);
    storage.vindex = ValidI;
  }
};

template<>
struct reconstruct_operation<trivial_reconstruction::move_assign> {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<ValidI, Ts...>::type, Args...>::value
  )
  {
    using T_i = typename extension::pack_indexing<ValidI, Ts...>::type;
    if (storage.index() == ValidI) {
      raw_get<ValidI>(storage.vunion) = T_i(std::forward<Args>(args)...);
    } else {
      using union_type = typename variant_storage<Ts...>::union_type;
      union_type tmp(in_place_index_t<ValidI>{}, std::forward<Args>(args)...);  // may throw
      storage.vunion = std::move(tmp);  // trivial move assign, won't throw
      storage.vindex = ValidI;
    }
  }
};

template<>
struct reconstruct_operation<trivial_reconstruction::copy_assign> {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<ValidI, Ts...>::type, Args...>::value
  )
  {
    using T_i = typename extension::pack_indexing<ValidI, Ts...>::type;
    if (storage.index() == ValidI) {
      T_i tmp(std::forward<Args>(args)...);
      raw_get<ValidI>(storage.vunion) = tmp;
    } else {
      using union_type = typename variant_storage<Ts...>::union_type;
      union_type tmp(in_place_index_t<ValidI>{}, std::forward<Args>(args)...);  // may throw
      storage.vunion = tmp;  // trivial copy assign, won't throw
      storage.vindex = ValidI;
    }
  }
};

struct construct_on_valueless_operation {
  template<std::size_t ValidI, class... Ts, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<ValidI, Ts...>::type, Args...>::value
  )
  {
    polyfill::construct_at(&storage.vunion, in_place_index_t<ValidI>{}, std::forward<Args>(args)...);
    storage.vindex = ValidI;
  }
};

template<class... Ts>
struct construct_on_valueless_visitor {
  variant_storage<Ts...>& storage;

  template<std::size_t J, class OtherContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, OtherContainedT&& other_value) noexcept(
      std::is_nothrow_constructible<typename remove_cvref<OtherContainedT>::type, OtherContainedT>::value
  )
  {
    no_op_wrapper<J, construct_on_valueless_operation>::apply(storage, std::forward<OtherContainedT>(other_value));
  }
};

template<bool NeverValueless>
struct emplace_operation;

template<>
struct emplace_operation</*NeverValueless = */ true> {
  template<std::size_t ValidI, class... Ts, class... Args, class T_i = typename extension::pack_indexing<ValidI, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR T_i& apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(std::is_nothrow_constructible<T_i, Args...>::value)
  {
    reconstruct_operation<variant_detail::select_trivial_reconstruction<T_i>()>::template apply<ValidI>(storage, std::forward<Args>(args)...);
    return raw_get<ValidI>(storage.vunion);
  }
};

template<>
struct emplace_operation</*NeverValueless = */ false> {
  template<std::size_t ValidI, class... Ts, class... Args, class T_i = typename extension::pack_indexing<ValidI, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR T_i& apply(variant_storage<Ts...>& storage, Args&&... args) noexcept(std::is_nothrow_constructible<T_i, Args...>::value)
  {
    storage.dynamic_reset();
    construct_on_valueless_operation::apply<ValidI>(storage, std::forward<Args>(args)...);  // may throw
    return raw_get<ValidI>(storage.vunion);
  }
};

template<bool IsDirectConstructionSaferThanMovingTemporary>
struct emplace_directly_or_move_temporary_impl;

template<>
struct emplace_directly_or_move_temporary_impl</*IsDirectConstructionSaferThanMovingTemporary = */ true> {
  template<std::size_t ValidJ, class... Ts, class Rhs, class T_j = typename extension::pack_indexing<ValidJ, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, Rhs&& rhs) noexcept(std::is_nothrow_constructible<T_j, Rhs>::value)
  {
    lhs.template emplace<ValidJ>(std::forward<Rhs>(rhs));
  }
};

template<>
struct emplace_directly_or_move_temporary_impl</*IsDirectConstructionSaferThanMovingTemporary = */ false> {
  template<std::size_t ValidJ, class... Ts, class Rhs, class T_j = typename extension::pack_indexing<ValidJ, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, Rhs&& rhs)
  {
    T_j temporary(std::forward<Rhs>(rhs));
    lhs.template emplace<ValidJ>(std::move(temporary));
  }
};

struct emplace_directly_or_move_temporary {
  template<std::size_t ValidJ, class... Ts, class Rhs, class T_j = typename extension::pack_indexing<ValidJ, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs_storage, Rhs&& rhs) noexcept(std::is_nothrow_constructible<T_j, Rhs>::value)
  {
    constexpr bool IsDirectConstructionSaferThanMovingTemporary =
        disjunction<std::is_nothrow_constructible<T_j, Rhs>, negation<std::is_nothrow_move_constructible<T_j>>>::value;
    emplace_directly_or_move_temporary_impl<IsDirectConstructionSaferThanMovingTemporary>::template apply<ValidJ>(lhs_storage, std::forward<Rhs>(rhs));
  }
};

struct copy_assign_operation {
  template<std::size_t ValidJ, class RhsContained, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained const& rhs_value) noexcept(
      std::is_nothrow_copy_assignable<RhsContained>::value && std::is_nothrow_copy_constructible<RhsContained>::value
  )
  {
    if (lhs.index() == ValidJ) {
      raw_get<ValidJ>(lhs.vunion) = rhs_value;
    } else {
      emplace_directly_or_move_temporary::apply<ValidJ>(lhs, rhs_value);
    }
  }
};

template<class... Ts>
struct copy_assign_visitor {
  variant_storage<Ts...>& lhs;

  template<std::size_t J, class RhsContained>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, RhsContained const& rhs_value) noexcept(
      std::is_nothrow_copy_assignable<RhsContained>::value && std::is_nothrow_copy_constructible<RhsContained>::value
  )
  {
    no_op_wrapper<J, copy_assign_operation>::apply(lhs, rhs_value);
  }
};

struct move_assign_operation {
  template<std::size_t ValidJ, class... Ts, class RhsContained, class T_j = typename extension::pack_indexing<ValidJ, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs, RhsContained&& rhs_value) noexcept(
      std::is_nothrow_move_assignable<T_j>::value && std::is_nothrow_move_constructible<T_j>::value
  )
  {
    if (lhs.index() == ValidJ) {
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
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<J>, RhsContained&& rhs_value) noexcept(
      std::is_nothrow_move_assignable<typename remove_cvref<RhsContained>::type>::value
      && std::is_nothrow_move_constructible<typename remove_cvref<RhsContained>::type>::value
  )
  {
    no_op_wrapper<J, move_assign_operation>::apply(lhs, rhs_value);
  }
};

struct generic_assign_operation {
  template<std::size_t ValidJ, class... Ts, class RhsT, class T_j = typename extension::pack_indexing<ValidJ, Ts...>::type>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant_storage<Ts...>& lhs_storage, RhsT&& rhs) noexcept(
      std::is_nothrow_assignable<T_j&, RhsT>::value && std::is_nothrow_constructible<T_j, RhsT>::value
  )
  {
    if (lhs_storage.index() == ValidJ) {
      raw_get<ValidJ>(lhs_storage.vunion) = std::forward<RhsT>(rhs);
    } else {
      emplace_directly_or_move_temporary::apply<ValidJ>(lhs_storage, std::forward<RhsT>(rhs));
    }
  }
};

template<class... Ts>
struct variant_storage {
  using union_type = typename variant_detail::make_variadic_union<Ts...>::type;
  using index_type = typename variant_detail::select_index<sizeof...(Ts)>::type;

  union_type vunion;
  index_type vindex;

  // Creates valueless state in order to being ready for copy/move construction
  explicit constexpr variant_storage() : vunion(valueless), vindex(variant_npos_for<sizeof...(Ts)>::value) {}

  template<std::size_t I, class... Args>
  constexpr explicit variant_storage(in_place_index_t<I> ipi, Args&&... args) noexcept(
      std::is_nothrow_constructible<typename extension::pack_indexing<I, Ts...>::type, Args...>::value
  )
      : vunion(ipi, std::forward<Args>(args)...), vindex(I)
  {
  }

  constexpr bool valueless_by_exception() const noexcept { return vindex == variant_npos_for<sizeof...(Ts)>::value; }
  constexpr std::size_t index() const noexcept { return valueless_by_exception() ? variant_npos : vindex; }

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, union_type&>::type raw_visit(Visitor&& vis) &
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), vunion, variant_detail::bias<union_type::never_valueless>(index()));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, union_type const&>::type raw_visit(Visitor&& vis) const&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), vunion, variant_detail::bias<union_type::never_valueless>(index()));
  }

  template<class Visitor>
  YK_POLYFILL_CXX14_CONSTEXPR typename raw_visit_result<Visitor, union_type&&>::type raw_visit(Visitor&& vis) &&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(vunion), variant_detail::bias<union_type::never_valueless>(index()));
  }

  template<class Visitor>
  constexpr typename raw_visit_result<Visitor, union_type const&&>::type raw_visit(Visitor&& vis) const&&
  {
    return raw_visit_dispatch::apply(std::forward<Visitor>(vis), std::move(vunion), variant_detail::bias<union_type::never_valueless>(index()));
  }

  // calls `visit` to destroy contained value and *DO NOT* set index
  YK_POLYFILL_CXX20_CONSTEXPR void dynamic_destroy() noexcept { raw_visit(destroy_visitor{}); }

  // calls `visit` to destroy contained value and *DO* set index
  YK_POLYFILL_CXX20_CONSTEXPR void dynamic_reset() noexcept { raw_visit(reset_visitor<Ts...>{*this}); }

  template<std::size_t ValidI, class... Args, class T_i = typename extension::pack_indexing<ValidI, Ts...>::type>
  YK_POLYFILL_CXX20_CONSTEXPR T_i& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T_i, Args...>::value)
  {
    return emplace_operation<union_type::never_valueless>::template apply<ValidI>(*this, std::forward<Args>(args)...);
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

  YK_POLYFILL_CXX20_CONSTEXPR void _move_assign(variant_storage&& other) noexcept(
      conjunction<std::is_nothrow_move_constructible<Ts>..., std::is_nothrow_move_assignable<Ts>...>::value
  )
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

  YK_POLYFILL_CXX20_CONSTEXPR ~variant_base() { this->dynamic_destroy(); }
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

struct swap_same_index_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(variant<Ts...>& other, ContainedT& lhs_val)
  {
    using std::swap;
    swap(lhs_val, polyfill::get<ValidI>(other));
  }
};

template<class... Ts>
struct swap_same_index_visitor {
  variant<Ts...>& other_ref;
  template<std::size_t I, class ContainedT>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(in_place_index_t<I>, ContainedT&& lhs_val) const
  {
    no_op_wrapper<I, swap_same_index_operation>::apply(other_ref, lhs_val);
  }
};

}  // namespace variant_detail

template<class... Ts>
class variant : private cond_trivial_smf<typename variant_detail::make_variant_base<Ts...>::type, Ts...> {
  static_assert(sizeof...(Ts) > 0, "variant must be instantiated with at least one type template parameter");

private:
  using base_type = cond_trivial_smf<typename variant_detail::make_variant_base<Ts...>::type, Ts...>;

public:
  using base_type::index;
  using base_type::valueless_by_exception;

  template<
      std::size_t I, class... Args, class SelectedType = typename variant_alternative<I, variant>::type,
      typename std::enable_if<std::is_constructible<SelectedType, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR SelectedType& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<SelectedType, Args...>::value)
  {
    return base_type::template emplace<I>(std::forward<Args>(args)...);
  }

  template<
      std::size_t I, class U, class... Args, class SelectedType = typename variant_alternative<I, variant>::type,
      typename std::enable_if<std::is_constructible<SelectedType, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR SelectedType& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<SelectedType, std::initializer_list<U>&, Args...>::value
  )
  {
    return base_type::template emplace<I>(il, std::forward<Args>(args)...);
  }

  template<
      class T, class... Args, typename std::enable_if<variant_detail::exactly_once<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
    return base_type::template emplace<variant_detail::find_index<T, Ts...>::value>(std::forward<Args>(args)...);
  }

  template<
      class T, class U, class... Args, typename std::enable_if<variant_detail::exactly_once<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value
  )
  {
    return base_type::template emplace<variant_detail::find_index<T, Ts...>::value>(il, std::forward<Args>(args)...);
  }

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

  template<
      class T, class... Args, typename std::enable_if<variant_detail::exactly_once<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit variant(in_place_type_t<T>, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : base_type(in_place_index_t<variant_detail::find_index<T, Ts...>::value>{}, std::forward<Args>(args)...)
  {
  }

  template<
      class T, typename std::enable_if<!std::is_same<typename remove_cvref<T>::type, variant>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<variant_detail::is_invocation_to_imaginary_function_set_valid<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      std::size_t SelectedIndex = variant_detail::select_alternative<T, Ts...>::value,
      class SelectedType = typename extension::pack_indexing<SelectedIndex, Ts...>::type,
      typename std::enable_if<conjunction<std::is_assignable<SelectedType&, T>, std::is_constructible<SelectedType, T>>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR variant& operator=(T&& t) noexcept(
      conjunction<std::is_nothrow_assignable<SelectedType&, T>, std::is_nothrow_constructible<SelectedType, T>>::value
  )
  {
    variant_detail::generic_assign_operation::apply<SelectedIndex>(*this, std::forward<T>(t));
    return *this;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void swap(variant& other) noexcept(conjunction<std::is_nothrow_move_constructible<Ts>..., is_nothrow_swappable<Ts>...>::value)
  {
    if (index() == other.index()) {
      if (!valueless_by_exception()) {
        this->raw_visit(variant_detail::swap_same_index_visitor<Ts...>{other});
      }
    } else {
      variant tmp(std::move(other));
      other = std::move(*this);
      *this = std::move(tmp);
    }
  }

  using base_type::raw_visit;

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

// holds_alternative

template<class T, class... Ts>
constexpr bool holds_alternative(variant<Ts...> const& v) noexcept
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return v.index() == variant_detail::find_index<T, Ts...>::value;
}

// get<T>

template<class T, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR T& get(variant<Ts...>& v)
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return polyfill::get<variant_detail::find_index<T, Ts...>::value>(v);
}

template<class T, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR T const& get(variant<Ts...> const& v)
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return polyfill::get<variant_detail::find_index<T, Ts...>::value>(v);
}

template<class T, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR T&& get(variant<Ts...>&& v)
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return polyfill::get<variant_detail::find_index<T, Ts...>::value>(std::move(v));
}

template<class T, class... Ts>
YK_POLYFILL_CXX14_CONSTEXPR T const&& get(variant<Ts...> const&& v)
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return polyfill::get<variant_detail::find_index<T, Ts...>::value>(std::move(v));
}

// get_if<I>

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX17_CONSTEXPR typename std::add_pointer<typename variant_alternative<I, variant<Ts...>>::type>::type get_if(variant<Ts...>* v) noexcept
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v && v->index() == I) {
    return std::addressof(polyfill::get<I>(*v));
  }
  return nullptr;
}

template<std::size_t I, class... Ts>
YK_POLYFILL_CXX17_CONSTEXPR typename std::add_pointer<typename variant_alternative<I, variant<Ts...>>::type const>::type get_if(
    variant<Ts...> const* v
) noexcept
{
  static_assert(I < sizeof...(Ts), "I must be in sizeof...(Ts)");
  if (v && v->index() == I) {
    return std::addressof(polyfill::get<I>(*v));
  }
  return nullptr;
}

// get_if<T>

template<class T, class... Ts>
YK_POLYFILL_CXX17_CONSTEXPR typename std::add_pointer<T>::type get_if(variant<Ts...>* v) noexcept
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return get_if<variant_detail::find_index<T, Ts...>::value>(v);
}

template<class T, class... Ts>
YK_POLYFILL_CXX17_CONSTEXPR typename std::add_pointer<T const>::type get_if(variant<Ts...> const* v) noexcept
{
  static_assert(variant_detail::exactly_once<T, Ts...>::value, "T must occur exactly once in Ts...");
  return get_if<variant_detail::find_index<T, Ts...>::value>(v);
}

// visit

namespace variant_detail {

// is_variant trait
template<class T>
struct is_variant : false_type {};

template<class... Ts>
struct is_variant<variant<Ts...>> : true_type {};

// product of variant sizes
template<class... Variants>
struct multi_visit_total_size;

template<>
struct multi_visit_total_size<> : integral_constant<std::size_t, 1> {};

template<class V0, class... Vs>
struct multi_visit_total_size<V0, Vs...> : integral_constant<std::size_t, variant_size<typename remove_cvref<V0>::type>::value * multi_visit_total_size<Vs...>::value> {};

// stride for K-th variant: product of sizes of variants after K
template<std::size_t K, class... Variants>
struct multi_visit_stride;

template<std::size_t K, class V0, class... Vs>
struct multi_visit_stride<K, V0, Vs...> : multi_visit_stride<K - 1, Vs...> {};

template<class V0, class... Vs>
struct multi_visit_stride<0, V0, Vs...> : multi_visit_total_size<Vs...> {};

// size of K-th variant in the pack
template<std::size_t K, class... Variants>
struct multi_visit_variant_size_at;

template<std::size_t K, class V0, class... Vs>
struct multi_visit_variant_size_at<K, V0, Vs...> : multi_visit_variant_size_at<K - 1, Vs...> {};

template<class V0, class... Vs>
struct multi_visit_variant_size_at<0, V0, Vs...> : variant_size<typename remove_cvref<V0>::type> {};

// index for K-th variant given flat index
template<std::size_t FlatI, std::size_t K, class... Variants>
struct multi_visit_index_at
    : integral_constant<std::size_t, (FlatI / multi_visit_stride<K, Variants...>::value) % multi_visit_variant_size_at<K, Variants...>::value> {};

// do_multi_visit: dispatch for a given flat index
template<class ReturnType, std::size_t FlatI, class IndexSeq, class Visitor, class... Variants>
struct do_multi_visit_impl;

template<class ReturnType, std::size_t FlatI, std::size_t... Ks, class Visitor, class... Variants>
struct do_multi_visit_impl<ReturnType, FlatI, index_sequence<Ks...>, Visitor, Variants...> {
  static YK_POLYFILL_CXX14_CONSTEXPR ReturnType call(Visitor&& vis, Variants&&... vars)
  {
    return polyfill::invoke(std::forward<Visitor>(vis), polyfill::get<multi_visit_index_at<FlatI, Ks, Variants...>::value>(std::forward<Variants>(vars))...);
  }
};

template<class ReturnType, std::size_t FlatI, class Visitor, class... Variants>
YK_POLYFILL_CXX14_CONSTEXPR ReturnType do_multi_visit(Visitor&& vis, Variants&&... vars)
{
  return do_multi_visit_impl<ReturnType, FlatI, index_sequence_for<Variants...>, Visitor, Variants...>::call(
      std::forward<Visitor>(vis), std::forward<Variants>(vars)...
  );
}

// visit function pointer type
template<class ReturnType, class Visitor, class... Variants>
using multi_visit_function_type = ReturnType (*)(Visitor&&, Variants&&...);

// visit table
template<class ReturnType, class Visitor, class IndexSeq, class... Variants>
struct multi_visit_table;

template<class ReturnType, class Visitor, std::size_t... FlatIs, class... Variants>
struct multi_visit_table<ReturnType, Visitor, index_sequence<FlatIs...>, Variants...> {
  static constexpr multi_visit_function_type<ReturnType, Visitor, Variants...> value[sizeof...(FlatIs)]{
      &do_multi_visit<ReturnType, FlatIs, Visitor, Variants...>...
  };
};

template<class ReturnType, class Visitor, std::size_t... FlatIs, class... Variants>
constexpr multi_visit_function_type<ReturnType, Visitor, Variants...>
    multi_visit_table<ReturnType, Visitor, index_sequence<FlatIs...>, Variants...>::value[sizeof...(FlatIs)];

// compute runtime flat index
inline constexpr std::size_t compute_flat_index_impl(std::size_t acc) { return acc; }

template<class V0, class... Vs>
constexpr std::size_t compute_flat_index_impl(std::size_t acc, V0 const& v0, Vs const&... vs)
{
  return variant_detail::compute_flat_index_impl(acc * variant_size<typename remove_cvref<V0>::type>::value + v0.index(), vs...);
}

template<class... Variants>
constexpr std::size_t compute_flat_index(Variants const&... vars)
{
  return variant_detail::compute_flat_index_impl(0, vars...);
}

// check any valueless
inline constexpr bool any_valueless_impl() { return false; }

template<class V0, class... Vs>
constexpr bool any_valueless_impl(V0 const& v0, Vs const&... vs)
{
  return v0.valueless_by_exception() || variant_detail::any_valueless_impl(vs...);
}

// return type deduction helper: invoke_result with get<0> of each variant
template<class Visitor, class... Variants>
using multi_visit_return_type = typename invoke_result<Visitor, decltype(polyfill::get<0>(std::declval<Variants>()))...>::type;

}  // namespace variant_detail

template<class Visitor, class... Variants, typename std::enable_if<conjunction<variant_detail::is_variant<typename remove_cvref<Variants>::type>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR variant_detail::multi_visit_return_type<Visitor, Variants...> visit(Visitor&& vis, Variants&&... vars)
{
  using return_type = variant_detail::multi_visit_return_type<Visitor, Variants...>;
  if (variant_detail::any_valueless_impl(vars...)) throw bad_variant_access{};
  constexpr std::size_t total = variant_detail::multi_visit_total_size<Variants...>::value;
  return variant_detail::multi_visit_table<return_type, Visitor, make_index_sequence<total>, Variants...>::value[variant_detail::compute_flat_index(vars...)](
      std::forward<Visitor>(vis), std::forward<Variants>(vars)...
  );
}

// swap

template<class... Ts, typename std::enable_if<conjunction<std::is_move_constructible<Ts>..., is_swappable<Ts>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX20_CONSTEXPR void swap(variant<Ts...>& lhs, variant<Ts...>& rhs) noexcept(
    conjunction<std::is_nothrow_move_constructible<Ts>..., is_nothrow_swappable<Ts>...>::value
)
{
  lhs.swap(rhs);
}

// monostate comparisons

constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }
constexpr bool operator<(monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>(monostate, monostate) noexcept { return false; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }

#if __cplusplus >= 202002L
constexpr std::strong_ordering operator<=>(monostate, monostate) noexcept { return std::strong_ordering::equal; }
#endif

// variant comparison operators

namespace variant_detail {

template<std::size_t I, class Operation, class ReturnType, ReturnType DefaultValue>
struct returning_op_wrapper {
  template<class... Args>
  static constexpr ReturnType apply(Args&&... args)
  {
    return Operation::template apply<I>(std::forward<Args>(args)...);
  }
};

template<class Operation, class ReturnType, ReturnType DefaultValue>
struct returning_op_wrapper<variant_npos, Operation, ReturnType, DefaultValue> {
  template<class... Args>
  static constexpr ReturnType apply(Args&&...) noexcept
  {
    return DefaultValue;
  }
};

struct eq_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val == polyfill::get<ValidI>(rhs);
  }
};

struct ne_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val != polyfill::get<ValidI>(rhs);
  }
};

struct lt_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val < polyfill::get<ValidI>(rhs);
  }
};

struct le_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val <= polyfill::get<ValidI>(rhs);
  }
};

struct gt_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val > polyfill::get<ValidI>(rhs);
  }
};

struct ge_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr bool apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val >= polyfill::get<ValidI>(rhs);
  }
};

template<class Operation, bool DefaultValue, class... Ts>
struct cmp_visitor {
  variant<Ts...> const& rhs_ref;
  template<std::size_t I, class ContainedT>
  constexpr bool operator()(in_place_index_t<I>, ContainedT&& lhs_val) const
  {
    return returning_op_wrapper<I, Operation, bool, DefaultValue>::apply(rhs_ref, lhs_val);
  }
};

template<class... Ts>
using eq_visitor = cmp_visitor<eq_operation, true, Ts...>;

template<class... Ts>
using ne_visitor = cmp_visitor<ne_operation, false, Ts...>;

template<class... Ts>
using lt_visitor = cmp_visitor<lt_operation, false, Ts...>;

template<class... Ts>
using le_visitor = cmp_visitor<le_operation, true, Ts...>;

template<class... Ts>
using gt_visitor = cmp_visitor<gt_operation, false, Ts...>;

template<class... Ts>
using ge_visitor = cmp_visitor<ge_operation, true, Ts...>;

#if __cplusplus >= 202002L

template<class ResultType>
struct three_way_operation {
  template<std::size_t ValidI, class ContainedT, class... Ts>
  static constexpr ResultType apply(variant<Ts...> const& rhs, ContainedT const& lhs_val)
  {
    return lhs_val <=> polyfill::get<ValidI>(rhs);
  }
};

template<class ResultType>
struct three_way_npos_operation {
  template<std::size_t /* variant_npos */, class... Args>
  static constexpr ResultType apply(Args&&...) noexcept
  {
    return ResultType::equivalent;
  }
};

template<class ResultType, class... Ts>
struct three_way_cmp_visitor {
  variant<Ts...> const& rhs_ref;
  template<std::size_t I, class ContainedT>
  constexpr ResultType operator()(in_place_index_t<I>, ContainedT&& lhs_val) const
  {
    if constexpr (I == variant_npos) {
      return ResultType::equivalent;
    } else {
      return lhs_val <=> polyfill::get<I>(rhs_ref);
    }
  }
};

#endif

}  // namespace variant_detail

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() == std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator==(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.index() != rhs.index()) return false;
  if (lhs.valueless_by_exception()) return true;
  return lhs.raw_visit(variant_detail::eq_visitor<Ts...>{rhs});
}

#if __cplusplus < 202002L

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() != std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.index() != rhs.index()) return true;
  if (lhs.valueless_by_exception()) return false;
  return lhs.raw_visit(variant_detail::ne_visitor<Ts...>{rhs});
}

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() < std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (rhs.valueless_by_exception()) return false;
  if (lhs.valueless_by_exception()) return true;
  if (lhs.index() != rhs.index()) return lhs.index() < rhs.index();
  return lhs.raw_visit(variant_detail::lt_visitor<Ts...>{rhs});
}

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() <= std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.valueless_by_exception()) return true;
  if (rhs.valueless_by_exception()) return false;
  if (lhs.index() != rhs.index()) return lhs.index() < rhs.index();
  return lhs.raw_visit(variant_detail::le_visitor<Ts...>{rhs});
}

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() > std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (lhs.valueless_by_exception()) return false;
  if (rhs.valueless_by_exception()) return true;
  if (lhs.index() != rhs.index()) return lhs.index() > rhs.index();
  return lhs.raw_visit(variant_detail::gt_visitor<Ts...>{rhs});
}

template<
    class... Ts,
    typename std::enable_if<
        conjunction<std::is_convertible<decltype(std::declval<Ts const&>() >= std::declval<Ts const&>()), bool>...>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>=(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  if (rhs.valueless_by_exception()) return true;
  if (lhs.valueless_by_exception()) return false;
  if (lhs.index() != rhs.index()) return lhs.index() > rhs.index();
  return lhs.raw_visit(variant_detail::ge_visitor<Ts...>{rhs});
}

#else  // C++20

// Three-way comparison for variant
template<class... Ts>
  requires (std::three_way_comparable<Ts> && ...)
constexpr std::common_comparison_category_t<std::compare_three_way_result_t<Ts>...> operator<=>(variant<Ts...> const& lhs, variant<Ts...> const& rhs)
{
  using result_type = std::common_comparison_category_t<std::compare_three_way_result_t<Ts>...>;
  if (lhs.valueless_by_exception() && rhs.valueless_by_exception()) return result_type::equivalent;
  if (lhs.valueless_by_exception()) return result_type::less;
  if (rhs.valueless_by_exception()) return result_type::greater;
  if (lhs.index() != rhs.index()) return lhs.index() <=> rhs.index();
  return lhs.raw_visit(variant_detail::three_way_cmp_visitor<result_type, Ts...>{rhs});
}

#endif

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_VARIANT_HPP
