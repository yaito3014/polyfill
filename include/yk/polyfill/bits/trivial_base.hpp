#ifndef YK_POLYFILL_BITS_TRIVIAL_BASE_HPP
#define YK_POLYFILL_BITS_TRIVIAL_BASE_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/config.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace trivial_base_detail {

template<class Base>
struct non_trivial_copy_constructor : Base {
  using Base::Base;

  non_trivial_copy_constructor() = default;

  constexpr non_trivial_copy_constructor(non_trivial_copy_constructor const& other) noexcept(noexcept(Base::construct_from(static_cast<Base const&>(other))))
  {
    Base::construct_from(static_cast<Base const&>(other));
  }

  non_trivial_copy_constructor(non_trivial_copy_constructor&&) = default;

  non_trivial_copy_constructor& operator=(non_trivial_copy_constructor const&) = default;

  non_trivial_copy_constructor& operator=(non_trivial_copy_constructor&&) = default;
};

template<class Base>
struct deleted_copy_constructor : Base {
  using Base::Base;

  deleted_copy_constructor() = default;
  deleted_copy_constructor(deleted_copy_constructor const&) = delete;
  deleted_copy_constructor(deleted_copy_constructor&&) = default;
  deleted_copy_constructor& operator=(deleted_copy_constructor const&) = default;
  deleted_copy_constructor& operator=(deleted_copy_constructor&&) = default;
};

template<class Base, class... Ts>
using select_base_for_copy_constructor = typename std::conditional<
    conjunction<std::is_trivially_copy_constructible<Ts>...>::value, Base,
    typename std::conditional<
        conjunction<std::is_copy_constructible<Ts>...>::value, non_trivial_copy_constructor<Base>, deleted_copy_constructor<Base>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_move_constructor : select_base_for_copy_constructor<Base, Ts...> {
  using Base::Base;

  non_trivial_move_constructor() = default;

  non_trivial_move_constructor(non_trivial_move_constructor const&) = default;

  constexpr non_trivial_move_constructor(non_trivial_move_constructor&& other) noexcept(noexcept(Base::construct_from(static_cast<Base&&>(other))))
  {
    Base::construct_from(static_cast<Base&&>(other));
  }

  non_trivial_move_constructor& operator=(non_trivial_move_constructor const&) = default;
  non_trivial_move_constructor& operator=(non_trivial_move_constructor&&) = default;
};

template<class Base, class... Ts>
struct deleted_move_constructor : select_base_for_copy_constructor<Base, Ts...> {
  using Base::Base;

  deleted_move_constructor() = default;
  deleted_move_constructor(deleted_move_constructor const&) = default;
  deleted_move_constructor(deleted_move_constructor&&) = delete;
  deleted_move_constructor& operator=(deleted_move_constructor const&) = default;
  deleted_move_constructor& operator=(deleted_move_constructor&&) = default;
};

template<class Base, class... Ts>
using select_base_for_constructors = typename std::conditional<
    conjunction<std::is_trivially_move_constructible<Ts>...>::value, Base,
    typename std::conditional<
        conjunction<std::is_move_constructible<Ts>...>::value, non_trivial_move_constructor<Base, Ts...>, deleted_move_constructor<Base, Ts...>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_copy_assignment : select_base_for_constructors<Base, Ts...> {
  using Base::Base;

  non_trivial_copy_assignment() = default;

  non_trivial_copy_assignment(non_trivial_copy_assignment const&) = default;

  non_trivial_copy_assignment(non_trivial_copy_assignment&&) = default;

  constexpr non_trivial_copy_assignment& operator=(non_trivial_copy_assignment const& other) noexcept(
      noexcept(Base::assign_from(static_cast<Base const&>(other)))
  )
  {
    Base::assign_from(static_cast<Base const&>(other));
  }

  non_trivial_copy_assignment& operator=(non_trivial_copy_assignment&&) = default;
};

template<class Base, class... Ts>
struct deleted_copy_assignment : select_base_for_constructors<Base, Ts...> {
  using Base::Base;

  deleted_copy_assignment() = default;
  deleted_copy_assignment(deleted_copy_assignment const&) = default;
  deleted_copy_assignment(deleted_copy_assignment&&) = default;
  deleted_copy_assignment& operator=(deleted_copy_assignment const&) = delete;
  deleted_copy_assignment& operator=(deleted_copy_assignment&&) = default;
};

template<class Base, class... Ts>
using select_base_for_constructors_and_copy_assignment = typename std::conditional<
    conjunction<std::is_trivially_copy_assignable<Ts>...>::value, Base,
    typename std::conditional<
        conjunction<std::is_copy_assignable<Ts>...>::value, non_trivial_copy_assignment<Base, Ts...>, deleted_copy_assignment<Base, Ts...>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_move_assignment : select_base_for_constructors_and_copy_assignment<Base, Ts...> {
  using Base::Base;

  non_trivial_move_assignment() = default;

  non_trivial_move_assignment(non_trivial_move_assignment const&) = default;

  non_trivial_move_assignment(non_trivial_move_assignment&&) = default;

  non_trivial_move_assignment& operator=(non_trivial_move_assignment const&) = default;

  constexpr non_trivial_move_assignment& operator=(non_trivial_move_assignment&& other) noexcept(noexcept(Base::assign_from(static_cast<Base&&>(other))))
  {
    Base::assign_from(static_cast<Base&&>(other));
  }
};

template<class Base, class... Ts>
struct deleted_move_assignment : select_base_for_constructors_and_copy_assignment<Base, Ts...> {
  using Base::Base;

  deleted_move_assignment() = default;
  deleted_move_assignment(deleted_move_assignment const&) = default;
  deleted_move_assignment(deleted_move_assignment&&) = default;
  deleted_move_assignment& operator=(deleted_move_assignment const&) = default;
  deleted_move_assignment& operator=(deleted_move_assignment&&) = delete;
};

template<class Base, class... Ts>
using select_base_for_special_member_functions = typename std::conditional<
    conjunction<std::is_trivially_move_assignable<Ts>...>::value, Base,
    typename std::conditional<
        conjunction<std::is_move_assignable<Ts>...>::value, non_trivial_move_assignment<Base, Ts...>, deleted_move_assignment<Base, Ts...>>::type>::type;

}  // namespace trivial_base_detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_BITS_TRIVIAL_BASE_HPP
