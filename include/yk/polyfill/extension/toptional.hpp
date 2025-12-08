#ifndef YK_POLYFILL_EXTENSION_TOPTIONAL_HPP
#define YK_POLYFILL_EXTENSION_TOPTIONAL_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/optional_common.hpp>

#include <yk/polyfill/extension/specialization_of.hpp>

#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>

#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace yk {

namespace polyfill {

namespace extension {

template<class T, class Traits>
class toptional;

namespace toptional_detail {

template<class T, class Traits, bool Const>
class toptional_iterator {
public:
  using iterator_category =
#if __cpp_lib_ranges >= 201911L
      std::contiguous_iterator_tag
#else
      std::random_access_iterator_tag
#endif
      ;
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = typename std::conditional<Const, T const*, T*>::type;
  using reference = typename std::conditional<Const, T const&, T&>::type;

  constexpr toptional_iterator() noexcept = default;

  constexpr reference operator*() const noexcept { return *ptr_; }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator& operator++() noexcept
  {
    ++ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator operator++(int) noexcept
  {
    toptional_iterator temporary = *this;
    ++*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator& operator--() noexcept
  {
    --ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator operator--(int) noexcept
  {
    toptional_iterator temporary = *this;
    --*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator& operator+=(difference_type n) noexcept
  {
    ptr_ += n;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR toptional_iterator& operator-=(difference_type n) noexcept
  {
    ptr_ -= n;
    return *this;
  }

  constexpr pointer operator->() const noexcept { return ptr_; }

  constexpr reference operator[](difference_type n) const noexcept { return ptr_[n]; }

  friend constexpr toptional_iterator operator+(toptional_iterator const& it, difference_type n) noexcept { return toptional_iterator{it.ptr_ + n}; }

  friend constexpr toptional_iterator operator+(difference_type n, toptional_iterator const& it) noexcept { return toptional_iterator{it.ptr_ + n}; }

  friend constexpr toptional_iterator operator-(toptional_iterator const& it, difference_type n) noexcept { return toptional_iterator{it.ptr_ - n}; }

  friend constexpr difference_type operator-(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ - b.ptr_; }

  friend constexpr bool operator==(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ == b.ptr_; }

  friend constexpr bool operator!=(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ != b.ptr_; }

  friend constexpr bool operator<(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ < b.ptr_; }

  friend constexpr bool operator<=(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ <= b.ptr_; }

  friend constexpr bool operator>(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ > b.ptr_; }

  friend constexpr bool operator>=(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ >= b.ptr_; }

#if __cplusplus >= 202002L
  friend constexpr std::strong_ordering operator<=>(toptional_iterator const& a, toptional_iterator const& b) noexcept { return a.ptr_ <=> b.ptr_; }
#endif

private:
  friend toptional<T, Traits>;

  constexpr toptional_iterator(pointer ptr) noexcept : ptr_(ptr) {}

private:
  pointer ptr_ = nullptr;
};

}  // namespace toptional_detail

class bad_toptional_initialization : public std::exception {
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
      class U, class UTraits,
      typename std::enable_if<
          std::is_constructible<T, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U, UTraits>>::value, std::nullptr_t>::type =
          nullptr>
  constexpr toptional(toptional<U, UTraits> const& other) : data(checked_construct(other.data))
  {
  }
  template<
      class U, class UTraits,
      typename std::enable_if<
          std::is_constructible<T, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U, UTraits>>::value, std::nullptr_t>::type = nullptr>
  constexpr toptional(toptional<U, UTraits>&& other) : data(checked_construct(std::move(other.data)))
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
    if (has_value()) {
      data = std::forward<U>(u);
      if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    } else {
      unchecked_emplace(checked_construct(std::forward<U>(u)));
    }
    return *this;
  }

  template<
      class U, class UTraits,
      typename std::enable_if<
          std::is_assignable<T&, U const&>::value && !optional_detail::converts_from_any_cvref<T, toptional<U, UTraits>>::value
              && !std::is_assignable<T&, toptional<U, UTraits>&>::value && !std::is_assignable<T&, toptional<U, UTraits> const&>::value
              && !std::is_assignable<T&, toptional<U, UTraits>&&>::value && !std::is_assignable<T&, toptional<U, UTraits> const&&>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U, UTraits> const& other)
  {
    if (has_value()) {
      data = other.data;
      if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    } else {
      unchecked_emplace(checked_construct(other.data));
    }
    return *this;
  }

  template<
      class U, class UTraits,
      typename std::enable_if<
          std::is_assignable<T&, U>::value && !optional_detail::converts_from_any_cvref<T, toptional<U, UTraits>>::value
              && !std::is_assignable<T&, toptional<U, UTraits>&>::value && !std::is_assignable<T&, toptional<U, UTraits> const&>::value
              && !std::is_assignable<T&, toptional<U, UTraits>&&>::value && !std::is_assignable<T&, toptional<U, UTraits> const&&>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional& operator=(toptional<U, UTraits>&& other)
  {
    if (has_value()) {
      data = std::move(other.data);
      if (!Traits::is_engaged(data)) throw bad_toptional_initialization{};
    } else {
      unchecked_emplace(checked_construct(std::move(other.data)));
    }
    return *this;
  }

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(Args&&... args)
  {
    T temp = checked_construct(std::forward<Args>(args)...);
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), std::move(temp));
#else
    new (std::addressof(data)) T(std::move(temp));
#endif
    return data;
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(std::initializer_list<U> il, Args&&... args)
  {
    T temp = checked_construct(il, std::forward<Args>(args)...);
    data.~T();
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(data), std::move(temp));
#else
    new (std::addressof(data)) T(std::move(temp));
#endif
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

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) & noexcept(is_nothrow_invocable<F, decltype(**this)>::value) ->
      typename remove_cvref<typename invoke_result<F, decltype(**this)>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, toptional>::value, "result type of F must be specialization of toptional");
    if (has_value()) {
      return invoke(std::forward<F>(f), **this);
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const& noexcept(is_nothrow_invocable<F, decltype(**this)>::value) ->
      typename remove_cvref<typename invoke_result<F, decltype(**this)>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, toptional>::value, "result type of F must be specialization of toptional");
    if (has_value()) {
      return invoke(std::forward<F>(f), **this);
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) && noexcept(is_nothrow_invocable<F, decltype(std::move(**this))>::value) ->
      typename remove_cvref<typename invoke_result<F, decltype(std::move(**this))>::type>::type
  {
    using U = typename invoke_result<F, decltype(std::move(**this))>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, toptional>::value, "result type of F must be specialization of toptional");
    if (has_value()) {
      return invoke(std::forward<F>(f), std::move(**this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const&& noexcept(is_nothrow_invocable<F, decltype(std::move(**this))>::value) ->
      typename remove_cvref<typename invoke_result<F, decltype(std::move(**this))>::type>::type
  {
    using U = typename invoke_result<F, decltype(std::move(**this))>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, toptional>::value, "result type of F must be specialization of toptional");
    if (has_value()) {
      return invoke(std::forward<F>(f), std::move(**this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<template<class...> class UTraits = non_zero_traits, class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) & noexcept(is_nothrow_invocable<F, decltype(**this)>::value)
      -> toptional<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type, UTraits<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type>>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type;
    static_assert(std::is_constructible<U, typename invoke_result<F, decltype(**this)>::type>::value, "result type of F must be copy/move constructible");
    if (has_value()) {
      return toptional<U, UTraits<U>>(invoke(std::forward<F>(f), **this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<template<class...> class UTraits = non_zero_traits, class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const& noexcept(is_nothrow_invocable<F, decltype(**this)>::value)
      -> toptional<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type, UTraits<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type>>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type;
    static_assert(std::is_constructible<U, typename invoke_result<F, decltype(**this)>::type>::value, "result type of F must be copy/move constructible");
    if (has_value()) {
      return toptional<U, UTraits<U>>(invoke(std::forward<F>(f), **this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<template<class...> class UTraits = non_zero_traits, class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) && noexcept(is_nothrow_invocable<F, decltype(std::move(**this))>::value)
      -> toptional<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type, UTraits<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type>>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type;
    static_assert(
        std::is_constructible<U, typename invoke_result<F, decltype(std::move(**this))>::type>::value, "result type of F must be copy/move constructible"
    );
    if (has_value()) {
      return toptional<U, UTraits<U>>(invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return nullopt_holder::value;
    }
  }

  template<template<class...> class UTraits = non_zero_traits, class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const&& noexcept(is_nothrow_invocable<F, decltype(std::move(**this))>::value)
      -> toptional<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type, UTraits<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type>>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type;
    static_assert(
        std::is_constructible<U, typename invoke_result<F, decltype(std::move(**this))>::type>::value, "result type of F must be copy/move constructible"
    );
    if (has_value()) {
      return toptional<U, UTraits<U>>(invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F, typename std::enable_if<conjunction<is_invocable<F>, std::is_copy_constructible<T>>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional or_else(F&& f) const& noexcept(is_nothrow_invocable<F>::value)
  {
    static_assert(
        std::is_same<typename remove_cvref<typename invoke_result<F>::type>::type, toptional>::value, "result type of F must be equal to toptional<T>"
    );
    if (has_value()) {
      return *this;
    } else {
      return invoke(std::forward<F>(f));
    }
  }

  template<class F, typename std::enable_if<conjunction<is_invocable<F>, std::is_move_constructible<T>>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR toptional or_else(F&& f) && noexcept(is_nothrow_invocable<F>::value)
  {
    static_assert(
        std::is_same<typename remove_cvref<typename invoke_result<F>::type>::type, toptional>::value, "result type of F must be equal to toptional<T>"
    );
    if (has_value()) {
      return std::move(*this);
    } else {
      return invoke(std::forward<F>(f));
    }
  }

  using iterator = toptional_detail::toptional_iterator<T, Traits, false>;
  using const_iterator = toptional_detail::toptional_iterator<T, Traits, true>;

  YK_POLYFILL_CXX17_CONSTEXPR iterator begin() noexcept { return iterator{std::addressof(data) + !has_value()}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{std::addressof(data) + !has_value()}; }

  YK_POLYFILL_CXX17_CONSTEXPR iterator end() noexcept { return begin() + has_value(); }
  constexpr const_iterator end() const noexcept { return begin() + has_value(); }

private:
  template<class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR T checked_construct(Args&&... args)
  {
    T value(std::forward<Args>(args)...);
    if (!Traits::is_engaged(value)) throw bad_toptional_initialization{};
    return value;
  }

  template<class U, class... Args>
  static YK_POLYFILL_CXX14_CONSTEXPR T checked_construct(std::initializer_list<U> il, Args&&... args)
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
