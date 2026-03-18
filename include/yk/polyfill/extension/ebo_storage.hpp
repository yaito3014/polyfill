#ifndef YK_ZZ_POLYFILL_EXTENSION_EBO_STORAGE_HPP
#define YK_ZZ_POLYFILL_EXTENSION_EBO_STORAGE_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/core_traits.hpp>

#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace extension {

template<class T>
struct can_ebo : std::integral_constant<
                     bool, std::is_empty<T>::value
#if __cplusplus >= 201402L
                               && !std::is_final<T>::value
#endif
                     > {
};

template<class T, bool = can_ebo<T>::value>
struct ebo_storage;

template<class T>
struct ebo_storage<T, /*IsEmptyBase = */ false> {
  T value_;
  ebo_storage() = default;

  template<
      class U = T, typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, ebo_storage>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit ebo_storage(U&& u) noexcept(std::is_nothrow_constructible<T, U>::value) : value_(std::forward<U>(u))
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR T& stored_value() noexcept { return value_; }
  constexpr T const& stored_value() const noexcept { return value_; }
};

template<class T>
struct ebo_storage<T, /*IsEmptyBase = */ true> : private T {
  ebo_storage() = default;

  template<
      class U = T, typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, ebo_storage>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit ebo_storage(U&& u) noexcept(std::is_nothrow_constructible<T, U>::value) : T(std::forward<U>(u))
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR T& stored_value() noexcept { return static_cast<T&>(*this); }
  constexpr T const& stored_value() const noexcept { return static_cast<T const&>(*this); }
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_EXTENSION_EBO_STORAGE_HPP
