#ifndef YK_POLYFILL_EXTENSION_EBO_STORAGE_HPP
#define YK_POLYFILL_EXTENSION_EBO_STORAGE_HPP

#include <yk/polyfill/config.hpp>

#include <type_traits>

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
struct ebo_storage<T, /*EBO=*/false> {
  T value_;
  ebo_storage() = default;
  constexpr explicit ebo_storage(T const& v) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(v) {}

  template<class U = T, typename std::enable_if<!std::is_reference<U>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit ebo_storage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value) : value_(static_cast<T&&>(v)) {}
  YK_POLYFILL_CXX14_CONSTEXPR T& stored_value() noexcept { return value_; }
  constexpr T const& stored_value() const noexcept { return value_; }
};

template<class T>
struct ebo_storage<T, /*EBO=*/true> : private T {
  ebo_storage() = default;
  constexpr explicit ebo_storage(T const& v) noexcept(std::is_nothrow_copy_constructible<T>::value) : T(v) {}
  constexpr explicit ebo_storage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value) : T(static_cast<T&&>(v)) {}
  YK_POLYFILL_CXX14_CONSTEXPR T& stored_value() noexcept { return static_cast<T&>(*this); }
  constexpr T const& stored_value() const noexcept { return static_cast<T const&>(*this); }
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_EBO_STORAGE_HPP
