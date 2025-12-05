#ifndef YK_POLYFILL_EXTENSION_TOPTIONAL_HPP
#define YK_POLYFILL_EXTENSION_TOPTIONAL_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/optional_common.hpp>

#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>

#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace extension {

class bad_toptional_initialization : std::exception {
  char const* what() const noexcept override { return "initializing toptional with tombstone value"; }
};

template<class T, class = void>
struct non_zero_traits {};

template<class T>
struct non_zero_traits<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
  static constexpr bool is_engaged(T const& x) noexcept { return x != nullptr; }
  static constexpr T tombstone_value() noexcept { return nullptr; }
};

template<class T>
struct non_zero_traits<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
  static constexpr bool is_engaged(T const& x) noexcept { return x != 0; }
  static constexpr T tombstone_value() noexcept { return T{0}; }
};

template<class T, class Traits = non_zero_traits<T>>
class toptional {
  static_assert(!std::is_reference<T>::value, "toptional doesn't support reference type");

public:
  using value_type = T;

  constexpr toptional() : toptional(nullopt_holder::value) {}

  constexpr toptional(nullopt_t) noexcept(noexcept(Traits::tombstone_value())) : data(Traits::tombstone_value()) {}

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit toptional(in_place_t, Args&&... args) : data(std::forward<Args>(args)...)
  {
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit toptional(in_place_t, std::initializer_list<U> il, Args&&... args) : data(il, std::forward<Args>(args)...)
  {
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<
          std::is_constructible<T, U>::value && !std::is_same<typename remove_cvref<T>::type, toptional>::value
              && !std::is_same<typename remove_cvref<T>::type, in_place_t>::value && std::is_convertible<U, T>::value,
          std::nullptr_t>::type = nullptr>
  constexpr toptional(U&& u) : data(std::forward<U>(u))
  {
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<
          std::is_constructible<T, U>::value && !std::is_same<typename remove_cvref<T>::type, toptional>::value
              && !std::is_same<typename remove_cvref<T>::type, in_place_t>::value && !std::is_convertible<U, T>::value,
          std::nullptr_t>::type = nullptr>
  constexpr explicit toptional(U&& u) : data(std::forward<U>(u))
  {
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
  }

  template<
      class U,
      typename std::enable_if<
          std::is_constructible<T, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value, std::nullptr_t>::type = nullptr>
  constexpr toptional(toptional<U> const& other) noexcept(std::is_nothrow_constructible<T, U const&>::value) : data(other.data)
  {
  }
  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value, std::nullptr_t>::type = nullptr>
  constexpr toptional(toptional<U>&& other) noexcept(std::is_nothrow_constructible<T, U>::value) : data(std::move(other.data))
  {
  }

  toptional(toptional const&) = default;
  toptional(toptional&&) = default;

  toptional& operator=(nullopt_t) noexcept(noexcept(Traits::tombstone_value()))
  {
    data = Traits::tombstone_value();
    return *this;
  }

  toptional& operator=(toptional const&) = default;
  toptional& operator=(toptional&&) = default;

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<
          !std::is_same<typename remove_cvref<U>::type, toptional>::value
              && conjunction<std::is_scalar<T>, std::is_same<U, typename std::decay<U>::type>>::value && std::is_assignable<T&, U>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(U&& u) noexcept(std::is_nothrow_assignable<T, U>::value)
  {
    data = std::forward<U>(u);
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   std::is_assignable<T&, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value
                       && !std::is_assignable<T&, toptional<U>&>::value && !std::is_assignable<T&, toptional<U> const&>::value
                       && !std::is_assignable<T&, toptional<U>&&>::value && !std::is_assignable<T&, toptional<U> const&&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U> const& other) noexcept(std::is_nothrow_assignable<T&, U const&>::value)
  {
    data = other.data;
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   std::is_assignable<T&, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value
                       && !std::is_assignable<T&, toptional<U>&>::value && !std::is_assignable<T&, toptional<U> const&>::value
                       && !std::is_assignable<T&, toptional<U>&&>::value && !std::is_assignable<T&, toptional<U> const&&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U>&& other) noexcept(std::is_nothrow_assignable<T&, U>::value)
  {
    data = std::move(other.data);
    return *this;
  }

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), std::forward<Args>(args)...);
#else
    new (std::addressof(data)) T(std::forward<Args>(args)...);
#endif
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value
  )
  {
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), il, std::forward<Args>(args)...);
#else
    new (std::addressof(data)) T(il, std::forward<Args>(args)...);
#endif
  }

  YK_POLYFILL_CXX14_CONSTEXPR void swap(toptional& other) noexcept(is_nothrow_swappable<T>::value)
  {
    using std::swap;
    swap(data, other.data);
  }

  YK_POLYFILL_CXX14_CONSTEXPR T* operator->() noexcept { return std::addressof(data); }
  constexpr T const* operator->() const noexcept { return std::addressof(data); }

  YK_POLYFILL_CXX14_CONSTEXPR T& operator*() & noexcept { return data; }
  constexpr T const& operator*() const& noexcept { return data; }
  YK_POLYFILL_CXX14_CONSTEXPR T&& operator*() && noexcept { return std::move(data); }
  constexpr T const&& operator*() const&& noexcept { return std::move(data); }

  constexpr explicit operator bool() const noexcept(noexcept(Traits::is_engaged(data))) { return Traits::is_engaged(data); }

  constexpr bool has_value() const noexcept(noexcept(Traits::is_engaged(data))) { return Traits::is_engaged(data); }

  YK_POLYFILL_CXX14_CONSTEXPR T& value() & { return has_value() ? **this : throw bad_optional_access{}; }
  constexpr T const& value() const& { return has_value() ? **this : throw bad_optional_access{}; }
  YK_POLYFILL_CXX14_CONSTEXPR T&& value() && { return has_value() ? **this : throw bad_optional_access{}; }
  constexpr T const&& value() const&& { return has_value() ? **this : throw bad_optional_access{}; }

  template<class U = typename std::remove_cv<T>::type>
  constexpr T value_or(U&& u) const&
  {
    return has_value() ? **this : static_cast<T>(std::forward<U>(u));
  }

  template<class U = typename std::remove_cv<T>::type>
  YK_POLYFILL_CXX14_CONSTEXPR T value_or(U&& u) &&
  {
    return has_value() ? **this : static_cast<T>(std::forward<U>(u));
  }

  YK_POLYFILL_CXX14_CONSTEXPR void reset() noexcept(noexcept(Traits::tombstone_value())) { emplace(Traits::tombstone_value()); }

  // TODO: add monadic operations
  // TODO: add iterator support

private:
  T data;
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_TOPTIONAL_HPP
