#ifndef YK_POLYFILL_EXTENSION_EBO_STORAGE_HPP
#define YK_POLYFILL_EXTENSION_EBO_STORAGE_HPP

// ebo_storage<T>: EBO (Empty Base Optimisation) wrapper for any type T.
//
// When T is empty and non-final, ebo_storage inherits from T privately so the
// compiler can apply the zero-size base optimisation — the wrapper itself
// costs no extra storage.  When EBO is not applicable (non-empty or final T)
// the value is held as a data member.
//
// Use stored_value() to access the contained object regardless of which
// specialisation is selected.
//
// Typical use case: holding an allocator, comparator, or other policy object
// as a base class instead of a member so that empty policies have zero cost.

#include <yk/polyfill/config.hpp>

#include <type_traits>

namespace yk {

namespace polyfill {

namespace extension {

// can_ebo<T>: true iff T qualifies for Empty Base Optimisation.
//
// std::is_final is a C++14 library feature; in C++11 mode we omit the check —
// a final empty T would cause a hard error at ebo_storage instantiation, but
// such a type is essentially non-existent in practice.
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

// Non-EBO path: T is stored as a data member.
template<class T>
struct ebo_storage<T, /*EBO=*/false> {
  T value_;
  ebo_storage() = default;
  constexpr explicit ebo_storage(T const& v) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(v) {}
  constexpr explicit ebo_storage(T&& v) noexcept(std::is_nothrow_move_constructible<T>::value) : value_(static_cast<T&&>(v)) {}
  YK_POLYFILL_CXX14_CONSTEXPR T& stored_value() noexcept { return value_; }
  constexpr T const& stored_value() const noexcept { return value_; }
};

// EBO path: T is an empty, non-final class — inherit from it privately.
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
