#ifndef YK_POLYFILL_COND_TRIVIAL_SMF_HPP
#define YK_POLYFILL_COND_TRIVIAL_SMF_HPP

// derived from https://github.com/iris-cpp/iris with permission from original author

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/type_traits.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace detail {

template<class Base>
struct non_trivial_CC : Base {
  using Base::Base;

  YK_POLYFILL_CXX14_CONSTEXPR non_trivial_CC(non_trivial_CC const& other) noexcept(noexcept(Base::_copy_construct(static_cast<Base const&>(other)))) : Base()
  {
    Base::_copy_construct(static_cast<Base const&>(other));
  }

  non_trivial_CC() = default;
  non_trivial_CC(non_trivial_CC&&) = default;
  non_trivial_CC& operator=(non_trivial_CC const&) = default;
  non_trivial_CC& operator=(non_trivial_CC&&) = default;
};

template<class Base>
struct deleted_CC : Base {
  using Base::Base;

  deleted_CC(deleted_CC const&) = delete;

  deleted_CC() = default;
  deleted_CC(deleted_CC&&) = default;
  deleted_CC& operator=(deleted_CC const&) = default;
  deleted_CC& operator=(deleted_CC&&) = default;
};

template<class Base, class... Ts>
using cond_CC = typename std::conditional<
    conjunction<std::is_trivially_copy_constructible<Ts>...>::value, Base,
    typename std::conditional<conjunction<std::is_copy_constructible<Ts>...>::value, non_trivial_CC<Base>, deleted_CC<Base>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_MC : cond_CC<Base, Ts...> {
  using RealBase = cond_CC<Base, Ts...>;
  using RealBase::RealBase;

  YK_POLYFILL_CXX14_CONSTEXPR non_trivial_MC(non_trivial_MC&& other) noexcept(noexcept(Base::_move_construct(static_cast<RealBase&&>(other))))
  {
    Base::_move_construct(static_cast<RealBase&&>(other));
  }

  non_trivial_MC() = default;
  non_trivial_MC(non_trivial_MC const&) = default;
  non_trivial_MC& operator=(non_trivial_MC const&) = default;
  non_trivial_MC& operator=(non_trivial_MC&&) = default;
};

template<class Base, class... Ts>
struct deleted_MC : cond_CC<Base, Ts...> {
  using RealBase = cond_CC<Base, Ts...>;
  using RealBase::RealBase;

  deleted_MC(deleted_MC&&) = delete;

  deleted_MC() = default;
  deleted_MC(deleted_MC const&) = default;
  deleted_MC& operator=(deleted_MC const&) = default;
  deleted_MC& operator=(deleted_MC&&) = default;
};

template<class Base, class... Ts>
using cond_MC = typename std::conditional<
    conjunction<std::is_trivially_move_constructible<Ts>...>::value, cond_CC<Base, Ts...>,
    typename std::conditional<conjunction<std::is_move_constructible<Ts>...>::value, non_trivial_MC<Base, Ts...>, deleted_MC<Base, Ts...>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_CA : cond_MC<Base, Ts...> {
  using RealBase = cond_MC<Base, Ts...>;
  using RealBase::RealBase;

  YK_POLYFILL_CXX14_CONSTEXPR non_trivial_CA& operator=(non_trivial_CA const& rhs) noexcept(noexcept(Base::_copy_assign(static_cast<RealBase const&>(rhs))))
  {
    Base::_copy_assign(static_cast<RealBase const&>(rhs));
    return *this;
  }

  non_trivial_CA() = default;
  non_trivial_CA(non_trivial_CA const&) = default;
  non_trivial_CA(non_trivial_CA&&) = default;
  non_trivial_CA& operator=(non_trivial_CA&&) = default;
};

template<class Base, class... Ts>
struct deleted_CA : cond_MC<Base, Ts...> {
  using RealBase = cond_MC<Base, Ts...>;
  using RealBase::RealBase;

  deleted_CA& operator=(deleted_CA const&) = delete;

  deleted_CA() = default;
  deleted_CA(deleted_CA const&) = default;
  deleted_CA(deleted_CA&&) = default;
  deleted_CA& operator=(deleted_CA&&) = default;
};

template<class Base, class... Ts>
using cond_CA = typename std::conditional<
    conjunction<std::is_trivially_destructible<Ts>..., std::is_trivially_copy_constructible<Ts>..., std::is_trivially_copy_assignable<Ts>...>::value,
    cond_MC<Base, Ts...>,
    typename std::conditional<
        conjunction<std::is_copy_constructible<Ts>..., std::is_copy_assignable<Ts>...>::value, non_trivial_CA<Base, Ts...>,
        deleted_CA<Base, Ts...>>::type>::type;

template<class Base, class... Ts>
struct non_trivial_MA : cond_CA<Base, Ts...> {
  using RealBase = cond_CA<Base, Ts...>;
  using RealBase::RealBase;

  YK_POLYFILL_CXX14_CONSTEXPR non_trivial_MA& operator=(non_trivial_MA&& rhs) noexcept(noexcept(Base::_move_assign(static_cast<RealBase&&>(rhs))))
  {
    Base::_move_assign(static_cast<RealBase&&>(rhs));
    return *this;
  }

  non_trivial_MA() = default;
  non_trivial_MA(non_trivial_MA const&) = default;
  non_trivial_MA(non_trivial_MA&&) = default;
  non_trivial_MA& operator=(non_trivial_MA const&) = default;
};

template<class Base, class... Ts>
struct deleted_MA : cond_CA<Base, Ts...> {
  using RealBase = cond_CA<Base, Ts...>;
  using RealBase::RealBase;

  deleted_MA& operator=(deleted_MA&&) = delete;

  deleted_MA() = default;
  deleted_MA(deleted_MA const&) = default;
  deleted_MA(deleted_MA&&) = default;
  deleted_MA& operator=(deleted_MA const&) = default;
};

template<class Base, class... Ts>
using cond_trivial_smf = typename std::conditional<
    conjunction<std::is_trivially_destructible<Ts>..., std::is_trivially_move_constructible<Ts>..., std::is_trivially_move_assignable<Ts>...>::value,
    cond_CA<Base, Ts...>,
    typename std::conditional<
        conjunction<std::is_move_constructible<Ts>..., std::is_move_assignable<Ts>...>::value, non_trivial_MA<Base, Ts...>,
        deleted_MA<Base, Ts...>>::type>::type;

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif
