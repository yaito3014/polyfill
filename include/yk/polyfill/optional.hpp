#ifndef YK_POLYFILL_OPTIONAL_HPP
#define YK_POLYFILL_OPTIONAL_HPP

#include <yk/polyfill/bits/trivial_base.hpp>
#include <yk/polyfill/utility.hpp>

#include <yk/polyfill/config.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace optional_detail {

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

  YK_POLYFILL_CXX20_CONSTEXPR void reset() noexcept
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

template<class T, bool = std::is_reference<T>::value>
struct optional_storage_base;

template<class T>
struct optional_storage_base<T, true> {  // T is a reference type
  typename std::remove_reference<T>::type* ptr;

  using value_type = T;

  constexpr optional_storage_base() noexcept : ptr(nullptr) {}

  template<class Arg>
  constexpr explicit optional_storage_base(in_place_t, Arg&& arg) noexcept : ptr(std::addressof(arg))
  {
  }

  constexpr void reset() noexcept { ptr = nullptr; }

  [[nodiscard]] constexpr bool has_value() const noexcept { return ptr != nullptr; }

  [[nodiscard]] constexpr value_type& operator*() const& noexcept { return *ptr; }
  [[nodiscard]] constexpr value_type&& operator*() const&& noexcept { return std::forward<value_type>(*ptr); }
};

template<class T>
struct optional_storage_base<T, false> : public optional_destruct_base<T> {  // T is NOT a reference type
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

  [[nodiscard]] constexpr value_type& operator*() & noexcept { return this->value; }
  [[nodiscard]] constexpr value_type const& operator*() const& noexcept { return this->value; }
  [[nodiscard]] constexpr value_type&& operator*() && noexcept { return std::move(this->value); }
  [[nodiscard]] constexpr value_type const&& operator*() const&& noexcept { return std::move(this->value); }
};

}  // namespace optional_detail

struct nullopt_t {
  explicit nullopt_t() = default;
};

struct nullopt_holder {
  static constexpr nullopt_t value{};
};

constexpr nullopt_t nullopt_holder::value;

#if __cpp_inline_variables >= 201606L

inline constexpr nullopt_t nullopt{};

#endif

template<class T>
class optional : private trivial_base_detail::select_base_for_special_member_functions<optional_detail::optional_storage_base<T>, T> {
private:
  using base = trivial_base_detail::select_base_for_special_member_functions<optional_detail::optional_storage_base<T>, T>;

  template<class>
  friend class optional;

public:
  using value_type = T;

  constexpr optional() noexcept {}
  constexpr optional(nullopt_t) noexcept {}

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr optional(in_place_t ip, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) : base(ip, std::forward<Args>(args)...)
  {
  }

  using base::has_value;

  using base::operator*;
};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_OPTIONAL_HPP
