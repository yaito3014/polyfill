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
  constexpr explicit toptional(in_place_t, Args&&... args) : data(checked_construct(std::forward<Args>(args)...))
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit toptional(in_place_t, std::initializer_list<U> il, Args&&... args) : data(checked_construct(il, std::forward<Args>(args)...))
  {
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<
          std::is_constructible<T, U>::value && !std::is_same<typename remove_cvref<T>::type, toptional>::value
              && !std::is_same<typename remove_cvref<T>::type, in_place_t>::value && std::is_convertible<U, T>::value,
          std::nullptr_t>::type = nullptr>
  constexpr toptional(U&& u) : data(checked_construct(std::forward<U>(u)))
  {
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<
          std::is_constructible<T, U>::value && !std::is_same<typename remove_cvref<T>::type, toptional>::value
              && !std::is_same<typename remove_cvref<T>::type, in_place_t>::value && !std::is_convertible<U, T>::value,
          std::nullptr_t>::type = nullptr>
  constexpr explicit toptional(U&& u) : data(checked_construct(std::forward<U>(u)))
  {
  }

  template<
      class U,
      typename std::enable_if<
          std::is_constructible<T, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value, std::nullptr_t>::type = nullptr>
  constexpr toptional(toptional<U> const& other) : data(checked_construct(other.data))
  {
  }
  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value, std::nullptr_t>::type = nullptr>
  constexpr toptional(toptional<U>&& other) : data(checked_construct(std::move(other.data)))
  {
  }

  toptional(toptional const&) = default;
  toptional(toptional&&) = default;

  toptional& operator=(nullopt_t) noexcept(noexcept(unchecked_emplace(Traits::tombstone_value())))
  {
    unchecked_emplace(Traits::tombstone_value());
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
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(U&& u)
  {
    data = std::forward<U>(u);
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   std::is_assignable<T&, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value
                       && !std::is_assignable<T&, toptional<U>&>::value && !std::is_assignable<T&, toptional<U> const&>::value
                       && !std::is_assignable<T&, toptional<U>&&>::value && !std::is_assignable<T&, toptional<U> const&&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U> const& other)
  {
    data = other.data;
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   std::is_assignable<T&, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U>>::value
                       && !std::is_assignable<T&, toptional<U>&>::value && !std::is_assignable<T&, toptional<U> const&>::value
                       && !std::is_assignable<T&, toptional<U>&&>::value && !std::is_assignable<T&, toptional<U> const&&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U>&& other)
  {
    data = std::move(other.data);
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    return *this;
  }

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(Args&&... args)
  {
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), std::forward<Args>(args)...);
#else
    new (std::addressof(data)) T(std::forward<Args>(args)...);
#endif
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    return data;
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(std::initializer_list<U> il, Args&&... args)
  {
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), il, std::forward<Args>(args)...);
#else
    new (std::addressof(data)) T(il, std::forward<Args>(args)...);
#endif
    if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    return data;
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

  YK_POLYFILL_CXX14_CONSTEXPR T& value() &
  {
    if (has_value()) {
      return **this;
    } else {
      throw bad_optional_access{};
    }
  }
  YK_POLYFILL_CXX14_CONSTEXPR T const& value() const&
  {
    if (has_value()) {
      return **this;
    } else {
      throw bad_optional_access{};
    }
  }
  YK_POLYFILL_CXX14_CONSTEXPR T&& value() &&
  {
    if (has_value()) {
      return std::move(**this);
    } else {
      throw bad_optional_access{};
    }
  }
  YK_POLYFILL_CXX14_CONSTEXPR T const&& value() const&&
  {
    if (has_value()) {
      return std::move(**this);
    } else {
      throw bad_optional_access{};
    }
  }

  template<class U = typename std::remove_cv<T>::type>
  YK_POLYFILL_CXX14_CONSTEXPR T value_or(U&& u) const&
  {
    static_assert(
        conjunction<std::is_copy_constructible<T>, std::is_convertible<U&&, T>>::value, "T must be copy constructible and the argument must be convertible to T"
    );
    if (has_value()) {
      return **this;
    } else {
      return std::forward<U>(u);
    }
  }

  template<class U = typename std::remove_cv<T>::type>
  YK_POLYFILL_CXX14_CONSTEXPR T value_or(U&& u) &&
  {
    static_assert(
        conjunction<std::is_move_constructible<T>, std::is_convertible<U&&, T>>::value, "T must be move constructible and the argument must be convertible to T"
    );
    if (has_value()) {
      return std::move(**this);
    } else {
      return std::forward<U>(u);
    }
  }

  YK_POLYFILL_CXX14_CONSTEXPR void reset() noexcept(noexcept(Traits::tombstone_value())) { unchecked_emplace(Traits::tombstone_value()); }

  // TODO: add monadic operations
  // TODO: add iterator support

private:
  template<class... Args>
  constexpr T checked_construct(Args&&... args)
  {
    T value(std::forward<Args>(args)...);
    if (!Traits::is_engaged(value)) throw bad_toptional_initialization{};
    return value;
  }

  template<class U, class... Args>
  constexpr T checked_construct(std::initializer_list<U> il, Args&&... args)
  {
    T value(il, std::forward<Args>(args)...);
    if (!Traits::is_engaged(value)) throw bad_toptional_initialization{};
    return value;
  }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void unchecked_emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), std::forward<Args>(args)...);
#else
    new (std::addressof(data)) T(std::forward<Args>(args)...);
#endif
  }

  T data;
};

}  // namespace extension

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_EXTENSION_TOPTIONAL_HPP
