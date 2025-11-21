#ifndef YK_POLYFILL_OPTIONAL_HPP
#define YK_POLYFILL_OPTIONAL_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/bits/trivial_base.hpp>

#include <yk/polyfill/extension/specialization_of.hpp>

#include <yk/polyfill/bits/optional_common.hpp>
#include <yk/polyfill/functional.hpp>
#include <yk/polyfill/type_traits.hpp>
#include <yk/polyfill/utility.hpp>

#include <yk/polyfill/config.hpp>

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

template<class T>
class optional;

namespace optional_detail {

template<class T, bool Const>
class optional_iterator {
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

  constexpr optional_iterator() noexcept = default;

  constexpr reference operator*() const noexcept { return *ptr_; }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator& operator++() noexcept
  {
    ++ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator operator++(int) noexcept
  {
    optional_iterator temporary = *this;
    ++*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator& operator--() noexcept
  {
    --ptr_;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator operator--(int) noexcept
  {
    optional_iterator temporary = *this;
    --*this;
    return temporary;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator& operator+=(difference_type n) noexcept
  {
    ptr_ += n;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional_iterator& operator-=(difference_type n) noexcept
  {
    ptr_ -= n;
    return *this;
  }

  constexpr reference operator[](difference_type n) const noexcept { return ptr_[n]; }

  friend constexpr optional_iterator operator+(optional_iterator const& it, difference_type n) noexcept { return optional_iterator{it.ptr_ + n}; }

  friend constexpr optional_iterator operator-(optional_iterator const& it, difference_type n) noexcept { return optional_iterator{it.ptr_ - n}; }

  friend constexpr difference_type operator-(optional_iterator const& a, optional_iterator const& b) noexcept { return a.ptr_ - b.ptr_; }

private:
  friend optional<T>;
  friend optional<T&>;

  constexpr optional_iterator(pointer ptr) noexcept : ptr_(ptr) {}

private:
  T* ptr_ = nullptr;
};

struct empty_type {};

template<class T, bool = std::is_trivially_destructible<T>::value>
struct optional_destruct_base;

template<class T>
struct optional_destruct_base<T, true> {  // T is trivially destructible
  union {
    empty_type dummy;
    T value;
  };
  bool engaged;

  constexpr optional_destruct_base() noexcept : dummy(), engaged(false) {}

  template<class... Args>
  constexpr explicit optional_destruct_base(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : value(std::forward<Args>(args)...), engaged(true)
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR void reset() noexcept
  {
    if (engaged) {
      engaged = false;
    }
  }
};

template<class T>
struct optional_destruct_base<T, false> {  // T is NOT trivially destructible
  union {
    empty_type dummy;
    T value;
  };
  bool engaged;

  constexpr optional_destruct_base() noexcept : dummy(), engaged(false) {}

  template<class... Args>
  constexpr explicit optional_destruct_base(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : value(std::forward<Args>(args)...), engaged(true)
  {
  }

  YK_POLYFILL_CXX20_CONSTEXPR ~optional_destruct_base() noexcept
  {
    if (engaged) {
      value.~T();
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void reset() noexcept
  {
    if (engaged) {
      value.~T();
      engaged = false;
    }
  }
};

template<class T>
struct optional_storage_base : public optional_destruct_base<T> {  // T is NOT a reference type
  using base = optional_destruct_base<T>;

  using value_type = T;

  using base::base;

  [[nodiscard]] constexpr bool has_value() const noexcept { return this->engaged; }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void construct(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(std::addressof(this->value), std::forward<Args>(args)...);
#else
    new (std::addressof(this->value)) T(std::forward<Args>(args)...);
#endif
    this->engaged = true;
  }

  template<class Arg>
  YK_POLYFILL_CXX20_CONSTEXPR void assign(Arg&& arg) noexcept(std::is_nothrow_assignable<T&, Arg>::value && std::is_nothrow_constructible<T, Arg>::value)
  {
    if (this->engaged) {
      this->value = std::forward<Arg>(arg);
    } else {
      construct(std::forward<Arg>(arg));
    }
  }

  template<class UOpt>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_from(UOpt&& other) noexcept(noexcept(construct(*std::forward<UOpt>(other))))
  {
    if (this->engaged) {
      construct(*std::forward<UOpt>(other));
    }
  }

  template<class UOpt>
  YK_POLYFILL_CXX20_CONSTEXPR void assign_from(UOpt&& other) noexcept(noexcept(assign(*std::forward<UOpt>(other))))
  {
    if (this->engaged) {
      assign(*std::forward<UOpt>(other));
    } else {
      this->reset();
    }
  }

  YK_POLYFILL_CXX17_CONSTEXPR T* operator->() noexcept { return std::addressof(this->value); }
  YK_POLYFILL_CXX17_CONSTEXPR T const* operator->() const noexcept { return std::addressof(this->value); }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR value_type& operator*() & noexcept { return this->value; }
  [[nodiscard]] constexpr value_type const& operator*() const& noexcept { return this->value; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR value_type&& operator*() && noexcept { return std::move(this->value); }
  [[nodiscard]] constexpr value_type const&& operator*() const&& noexcept { return std::move(this->value); }
};

template<class T, class U>
struct is_eligible_for_construction {
  static constexpr bool value =
      std::is_constructible<T, U>::value && !std::is_same<typename remove_cvref<U>::type, in_place_t>::value
      && !std::is_same<typename remove_cvref<U>::type, optional<T>>::value
      && !(std::is_same<typename std::remove_cv<T>::type, bool>::value && extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value);
};

template<class T, class U>
struct allow_unwrapping {
  static constexpr bool value = std::is_same<typename std::remove_cv<T>::type, bool>::value || !converts_from_any_cvref<T, optional<U>>::value;
};

template<class T, class U>
struct allow_unwrapping_assignment {
  static constexpr bool value = !disjunction<
      converts_from_any_cvref<T, optional<U>>, std::is_assignable<T&, optional<U>&>, std::is_assignable<T&, optional<U>&&>,
      std::is_assignable<T&, optional<U> const&>, std::is_assignable<T&, optional<U> const&&>>::value;
};

}  // namespace optional_detail

template<class T>
class optional : private trivial_base_detail::select_base_for_special_member_functions<optional_detail::optional_storage_base<T>, T> {
private:
  using base = trivial_base_detail::select_base_for_special_member_functions<optional_detail::optional_storage_base<T>, T>;

  template<class>
  friend class optional;

public:
  using value_type = T;

  using base::has_value;
  using base::reset;
  using base::operator*;
  using base::operator->;

  constexpr optional() noexcept {}
  constexpr optional(nullopt_t) noexcept {}

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr optional(in_place_t ip, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) : base(ip, std::forward<Args>(args)...)
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr optional(in_place_t ip, std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value
  )
      : base(ip, il, std::forward<Args>(args)...)
  {
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<optional_detail::is_eligible_for_construction<T, U>::value && std::is_convertible<U, T>::value, std::nullptr_t>::type = nullptr>
  constexpr optional(U&& v) noexcept(std::is_nothrow_constructible<T, U>::value) : base(in_place_holder::value, std::forward<U>(v))
  {
  }

  template<
      class U = typename std::remove_cv<T>::type,
      typename std::enable_if<optional_detail::is_eligible_for_construction<T, U>::value && !std::is_convertible<U, T>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit optional(U&& v) noexcept(std::is_nothrow_constructible<T, U>::value) : base(in_place_holder::value, std::forward<U>(v))
  {
  }

  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && optional_detail::allow_unwrapping<T, U>::value && std::is_convertible<U const&, T>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR optional(optional<U> const& rhs) noexcept(std::is_nothrow_constructible<T, U const&>::value)
  {
    if (rhs.has_value()) {
      this->construct(*rhs);
    }
  }

  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && optional_detail::allow_unwrapping<T, U>::value && !std::is_convertible<U const&, T>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit optional(optional<U> const& rhs) noexcept(std::is_nothrow_constructible<T, U const&>::value)
  {
    if (rhs.has_value()) {
      this->construct(*rhs);
    }
  }

  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && optional_detail::allow_unwrapping<T, U>::value && std::is_convertible<U, T>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR optional(optional<U>&& rhs) noexcept(std::is_nothrow_constructible<T, U>::value)
  {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    }
  }

  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && optional_detail::allow_unwrapping<T, U>::value && !std::is_convertible<U, T>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit optional(optional<U>&& rhs) noexcept(std::is_nothrow_constructible<T, U>::value)
  {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR optional& operator=(nullopt_t) noexcept
  {
    reset();
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename remove_cvref<U>::type, optional>::value && std::is_constructible<T, U>::value && std::is_assignable<T&, U>::value
                       && !conjunction<std::is_scalar<U>, std::is_same<T, typename std::decay<T>::type>>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR optional& operator=(U&& v) noexcept(std::is_nothrow_constructible<T, U>::value && std::is_nothrow_assignable<T&, U>::value)
  {
    this->assign(std::forward<U>(v));
    return *this;
  }

  template<
      class U,
      typename std::enable_if<
          std::is_constructible<T, U const&>::value && std::is_assignable<T&, U const&>::value && optional_detail::allow_unwrapping_assignment<T, U>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR optional& operator=(optional<U> const& rhs) noexcept(
      conjunction<std::is_nothrow_constructible<T, U const&>, std::is_nothrow_assignable<T&, U const&>>::value
  )
  {
    if (rhs.has_value()) {
      this->assign(*rhs);
    } else {
      reset();
    }
    return *this;
  }

  template<
      class U, typename std::enable_if<
                   std::is_constructible<T, U>::value && std::is_assignable<T&, U>::value && optional_detail::allow_unwrapping_assignment<T, U>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR optional& operator=(optional<U>&& rhs) noexcept(
      conjunction<std::is_nothrow_constructible<T, U>, std::is_nothrow_assignable<T&, U>>::value
  )
  {
    if (rhs.has_value()) {
      this->assign(std::move(*rhs));
    } else {
      reset();
    }
    return *this;
  }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
    static_assert(std::is_constructible<T, Args...>::value, "T must be constructible from arguments");
    reset();
    this->construct(std::forward<Args>(args)...);
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR void emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value
  )
  {
    static_assert(std::is_constructible<T, Args...>::value, "T must be constructible from arguments");
    reset();
    this->construct(il, std::forward<Args>(args)...);
  }

  YK_POLYFILL_CXX20_CONSTEXPR void swap(optional& other) noexcept(conjunction<std::is_nothrow_move_constructible<T>, is_nothrow_swappable<T>>::value)
  {
    static_assert(std::is_move_constructible<T>::value, "T must be move constructible");
    if (has_value() == other.has_value()) {
      if (has_value()) {
        using std::swap;
        swap(**this, *other);
      }
    } else {
      optional& source = has_value() ? *this : other;
      optional& target = has_value() ? other : *this;
      target.construct(std::move(*source));
      source.reset();
    }
  }

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
  constexpr T value_or(U&& v) const& noexcept(std::is_nothrow_copy_constructible<T>::value && std::is_nothrow_constructible<T, U>::value)
  {
    static_assert(
        std::is_copy_constructible<T>::value && std::is_convertible<U&&, T>::value, "either T must be copy constructible or argument must be convertible to T"
    );
    return has_value() ? **this : static_cast<T>(std::forward<U>(v));
  }

  template<class U = typename std::remove_cv<T>::type>
  constexpr T value_or(U&& v) && noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_constructible<T, U>::value)
  {
    static_assert(
        std::is_move_constructible<T>::value && std::is_convertible<U&&, T>::value, "either T must be move constructible or argument must be convertible to T"
    );
    return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(v));
  }

  constexpr explicit operator bool() const noexcept { return has_value(); }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) & -> typename remove_cvref<typename invoke_result<F, decltype(**this)>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value, "result type of F must be specialization of optional");
    if (has_value()) {
      return invoke(std::forward<F>(f), **this);
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const& -> typename remove_cvref<typename invoke_result<F, decltype(**this)>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value, "result type of F must be specialization of optional");
    if (has_value()) {
      return invoke(std::forward<F>(f), **this);
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) && -> typename remove_cvref<typename invoke_result<F, decltype(std::move(**this))>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value, "result type of F must be specialization of optional");
    if (has_value()) {
      return invoke(std::forward<F>(f), std::move(**this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const&& -> typename remove_cvref<typename invoke_result<F, decltype(std::move(**this))>::type>::type
  {
    using U = typename invoke_result<F, decltype(**this)>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value, "result type of F must be specialization of optional");
    if (has_value()) {
      return invoke(std::forward<F>(f), std::move(**this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) & -> optional<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type;
    static_assert(std::is_constructible<U, typename invoke_result<F, decltype(**this)>::type>::value, "result type of F must be copy/move constructible");
    if (has_value()) {
      return optional<U>(invoke(std::forward<F>(f), **this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const& -> optional<typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(**this)>::type>::type;
    static_assert(std::is_constructible<U, typename invoke_result<F, decltype(**this)>::type>::value, "result type of F must be copy/move constructible");
    if (has_value()) {
      return optional<U>(invoke(std::forward<F>(f), **this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) && -> optional<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type;
    static_assert(
        std::is_constructible<U, typename invoke_result<F, decltype(std::move(**this))>::type>::value, "result type of F must be copy/move constructible"
    );
    if (has_value()) {
      return optional<U>(invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(
      F&& f
  ) const&& -> optional<typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type>
  {
    using U = typename std::remove_cv<typename invoke_result<F, decltype(std::move(**this))>::type>::type;
    static_assert(
        std::is_constructible<U, typename invoke_result<F, decltype(std::move(**this))>::type>::value, "result type of F must be copy/move constructible"
    );
    if (has_value()) {
      return optional<U>(invoke(std::forward<F>(f), std::move(**this)));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F, typename std::enable_if<conjunction<is_invocable<F>, std::is_copy_constructible<T>>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR optional or_else(F&& f) const&
  {
    static_assert(std::is_same<typename remove_cvref<typename invoke_result<F>::type>::type, optional>::value, "result type of F must be equal to optional<T>");
    if (has_value()) {
      return *this;
    } else {
      return invoke(std::forward<F>(f));
    }
  }

  template<class F, typename std::enable_if<conjunction<is_invocable<F>, std::is_move_constructible<T>>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR optional or_else(F&& f) &&
  {
    static_assert(std::is_same<typename remove_cvref<typename invoke_result<F>::type>::type, optional>::value, "result type of F must be equal to optional<T>");
    if (has_value()) {
      return std::move(*this);
    } else {
      return invoke(std::forward<F>(f));
    }
  }

  using iterator = optional_detail::optional_iterator<T, false>;
  using const_iterator = optional_detail::optional_iterator<T, true>;

  YK_POLYFILL_CXX14_CONSTEXPR iterator begin() noexcept { return iterator{operator->() + !has_value()}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{operator->() + !has_value()}; }

  YK_POLYFILL_CXX14_CONSTEXPR iterator end() noexcept { return begin() + has_value(); }
  constexpr const_iterator end() const noexcept { return begin() + has_value(); }
};

template<class T>
class optional<T&> {
public:
  constexpr optional() noexcept = default;
  constexpr optional(nullopt_t) noexcept : optional() {}
  constexpr optional(optional const&) noexcept = default;

  template<class Arg, typename std::enable_if<std::is_constructible<T&, Arg>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(in_place_t, Arg&& arg) noexcept(std::is_nothrow_constructible<T&, Arg>::value)
  {
    convert_ref_init_val(std::forward<Arg>(arg));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename remove_cvref<U>::type, optional>::value && !std::is_same<typename remove_cvref<U>::type, in_place_t>::value
                       && std::is_constructible<T&, U>::value && std::is_convertible<U, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(U&& u) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    convert_ref_init_val(std::forward<U>(u));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename remove_cvref<U>::type, optional>::value && !std::is_same<typename remove_cvref<U>::type, in_place_t>::value
                       && std::is_constructible<T&, U>::value && !std::is_convertible<U, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR explicit optional(U&& u) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    convert_ref_init_val(std::forward<U>(u));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value && std::is_constructible<T&, U&>::value
                       && std::is_convertible<U&, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(optional<U>& rhs) noexcept(std::is_nothrow_constructible<T&, U&>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*rhs);
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value && std::is_constructible<T&, U&>::value
                       && !std::is_convertible<U&, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR explicit optional(optional<U>& rhs) noexcept(std::is_nothrow_constructible<T&, U&>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*rhs);
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value
                       && std::is_constructible<T&, U const&>::value && std::is_convertible<U const&, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(optional<U> const& rhs) noexcept(std::is_nothrow_constructible<T&, U const&>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*rhs);
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value
                       && std::is_constructible<T&, U const&>::value && !std::is_convertible<U const&, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR explicit optional(optional<U> const& rhs) noexcept(std::is_nothrow_constructible<T&, U const&>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*rhs);
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value && std::is_constructible<T&, U>::value
                       && std::is_convertible<U, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(optional<U>&& rhs) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*std::move(rhs));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value && std::is_constructible<T&, U>::value
                       && !std::is_convertible<U, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR explicit optional(optional<U>&& rhs) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*std::move(rhs));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value
                       && std::is_constructible<T&, U const>::value && std::is_convertible<U const, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR optional(optional<U> const&& rhs) noexcept(std::is_nothrow_constructible<T&, U const>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*std::move(rhs));
  }

  template<
      class U, typename std::enable_if<
                   !std::is_same<typename std::remove_cv<T>::type, optional<U>>::value && !std::is_same<T&, U>::value
                       && std::is_constructible<T&, U const>::value && !std::is_convertible<U const, T&>::value,
                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR explicit optional(optional<U> const&& rhs) noexcept(std::is_nothrow_constructible<T&, U const>::value)
  {
    if (rhs.has_value()) convert_ref_init_val(*std::move(rhs));
  }

  YK_POLYFILL_CXX20_CONSTEXPR ~optional() = default;

  YK_POLYFILL_CXX14_CONSTEXPR optional& operator=(nullopt_t) noexcept
  {
    ptr = nullptr;
    return *this;
  }

  YK_POLYFILL_CXX14_CONSTEXPR optional& operator=(optional const&) noexcept = default;

  template<class U, typename std::enable_if<std::is_constructible<T&, U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX17_CONSTEXPR T& emplace(U&& u) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    convert_ref_init_val(std::forward<U>(u));
    return **this;
  }

  YK_POLYFILL_CXX17_CONSTEXPR void swap(optional& rhs) noexcept { std::swap(ptr, rhs.ptr); }

  constexpr T* operator->() const noexcept { return ptr; };

  constexpr T& operator*() const noexcept { return *ptr; }

  constexpr operator bool() const noexcept { return ptr != nullptr; }

  constexpr bool has_value() const noexcept { return ptr != nullptr; }

  YK_POLYFILL_CXX14_CONSTEXPR T& value() const
  {
    if (has_value()) {
      return **this;
    } else {
      throw bad_optional_access{};
    }
  }

  template<class U = typename std::remove_cv<T>::type>
  constexpr typename std::remove_cv<T>::type value_or(U&& u) const
  {
    static_assert(
        std::is_constructible<typename std::remove_cv<T>::type, T&>::value && std::is_convertible<U, typename std::remove_cv<T>::type>::value,
        "argument must be convertible to T"
    );
    return has_value() ? **this : static_cast<typename std::remove_cv<T>::type>(std::forward<U>(u));
  }

  YK_POLYFILL_CXX14_CONSTEXPR void reset() { ptr = nullptr; }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const -> typename remove_cvref<typename invoke_result<F, T&>::type>::type
  {
    using U = typename invoke_result<F, T&>::type;
    static_assert(extension::is_specialization_of<typename remove_cvref<U>::type, optional>::value, "result of F must be specialization of optional");
    if (has_value()) {
      return invoke(std::forward<F>(f), **this);
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const -> optional<typename std::remove_cv<typename invoke_result<F, T&>::type>::type>
  {
    using U = typename std::remove_cv<typename invoke_result<F, T&>::type>::type;
    if (has_value()) {
      return optional<U>(invoke(std::forward<F>(f), **this));
    } else {
      return nullopt_holder::value;
    }
  }

  template<class F, typename std::enable_if<is_invocable<F>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR optional or_else(F&& f) const
  {
    static_assert(
        std::is_same<typename remove_cvref<typename invoke_result<F>::type>::type, optional>::value, "result type of F must be equal to optional<T&>"
    );
    if (has_value()) {
      return *this;
    } else {
      return invoke(std::forward<F>(f));
    }
  }

  using iterator = optional_detail::optional_iterator<T, false>;
  using const_iterator = optional_detail::optional_iterator<T, true>;

  YK_POLYFILL_CXX14_CONSTEXPR iterator begin() noexcept { return iterator{operator->() + !has_value()}; }
  constexpr const_iterator begin() const noexcept { return const_iterator{operator->() + !has_value()}; }

  YK_POLYFILL_CXX14_CONSTEXPR iterator end() noexcept { return begin() + has_value(); }
  constexpr const_iterator end() const noexcept { return begin() + has_value(); }

private:
  template<class U>
  YK_POLYFILL_CXX17_CONSTEXPR void convert_ref_init_val(U&& u) noexcept(std::is_nothrow_constructible<T&, U>::value)
  {
    T& r(std::forward<U>(u));
    ptr = std::addressof(r);
  }

private:
  T* ptr = nullptr;
};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_OPTIONAL_HPP
