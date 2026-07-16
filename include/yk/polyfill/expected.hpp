#ifndef YK_ZZ_POLYFILL_EXPECTED_HPP
#define YK_ZZ_POLYFILL_EXPECTED_HPP

#include <yk/polyfill/bits/cond_trivial_smf.hpp>
#include <yk/polyfill/bits/core_traits.hpp>

#include <yk/polyfill/extension/specialization_of.hpp>

#include <yk/polyfill/bits/optional_common.hpp>  // detail::converts_from_any_cvref
#include <yk/polyfill/functional.hpp>            // invoke, invoke_result, is_invocable
#include <yk/polyfill/memory.hpp>                // construct_at
#include <yk/polyfill/type_traits.hpp>           // is_swappable, is_nothrow_swappable
#include <yk/polyfill/utility.hpp>               // in_place_t, in_place

#include <yk/polyfill/config.hpp>

#include <exception>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

template<class E>
class unexpected;

template<class T, class E>
class expected;

// [expected.unexpect]
struct unexpect_t {
  explicit unexpect_t() = default;
};

YK_POLYFILL_INLINE constexpr unexpect_t unexpect{};

// [expected.bad] bad_expected_access
template<class E>
class bad_expected_access;

template<>
class bad_expected_access<void> : public std::exception {
protected:
  bad_expected_access() noexcept = default;
  bad_expected_access(bad_expected_access const&) = default;
  bad_expected_access(bad_expected_access&&) = default;
  bad_expected_access& operator=(bad_expected_access const&) = default;
  bad_expected_access& operator=(bad_expected_access&&) = default;
  ~bad_expected_access() override = default;

public:
  char const* what() const noexcept override { return "bad access to expected without expected value"; }
};

template<class E>
class bad_expected_access : public bad_expected_access<void> {
public:
  explicit bad_expected_access(E e) : error_(std::move(e)) {}

  char const* what() const noexcept override { return "bad access to expected without expected value"; }

  YK_POLYFILL_NODISCARD E& error() & noexcept { return error_; }
  YK_POLYFILL_NODISCARD E const& error() const& noexcept { return error_; }
  YK_POLYFILL_NODISCARD E&& error() && noexcept { return std::move(error_); }
  YK_POLYFILL_NODISCARD E const&& error() const&& noexcept { return std::move(error_); }

private:
  E error_;
};

// [expected.un] unexpected
template<class E>
class unexpected {
  static_assert(std::is_object<E>::value, "E must be an object type");
  static_assert(!std::is_array<E>::value, "E must not be an array type");
  static_assert(!std::is_const<E>::value && !std::is_volatile<E>::value, "E must not be cv-qualified");
  static_assert(!extension::is_specialization_of<E, unexpected>::value, "E must not be a specialization of unexpected");

public:
  // copy/move constructors and assignment operators are implicit (a templated constructor
  // does not suppress them); leaving them implicit keeps them constexpr where E permits
  // without tripping C++11's rule that a constexpr member function is implicitly const.

  template<class Err = E,
           typename std::enable_if<!std::is_same<typename remove_cvref<Err>::type, unexpected>::value
                                       && !std::is_same<typename remove_cvref<Err>::type, in_place_t>::value && std::is_constructible<E, Err>::value,
                                   std::nullptr_t>::type = nullptr>
  constexpr explicit unexpected(Err&& e) noexcept(std::is_nothrow_constructible<E, Err>::value) : error_(std::forward<Err>(e))
  {
  }

  template<class... Args, typename std::enable_if<std::is_constructible<E, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit unexpected(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value) : error_(std::forward<Args>(args)...)
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<E, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit unexpected(in_place_t, std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Args...>::value)
      : error_(il, std::forward<Args>(args)...)
  {
  }

  YK_POLYFILL_NODISCARD constexpr E const& error() const& noexcept { return error_; }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E& error() & noexcept { return error_; }
  YK_POLYFILL_NODISCARD constexpr E const&& error() const&& noexcept { return std::move(error_); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E&& error() && noexcept { return std::move(error_); }

  template<class E2 = E, typename std::enable_if<is_swappable<E2>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR void swap(unexpected& other) noexcept(is_nothrow_swappable<E>::value)
  {
    using std::swap;
    swap(error_, other.error_);
  }

  template<class E2>
  friend constexpr bool operator==(unexpected const& lhs, unexpected<E2> const& rhs) noexcept(noexcept(lhs.error() == rhs.error()))
  {
    return lhs.error() == rhs.error();
  }

#if __cplusplus < 202002L
  template<class E2>
  friend constexpr bool operator!=(unexpected const& lhs, unexpected<E2> const& rhs) noexcept(noexcept(lhs.error() != rhs.error()))
  {
    return lhs.error() != rhs.error();
  }
#endif

  template<class E2 = E, typename std::enable_if<is_swappable<E2>::value, std::nullptr_t>::type = nullptr>
  friend YK_POLYFILL_CXX14_CONSTEXPR void swap(unexpected& lhs, unexpected& rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

private:
  E error_;
};

#if __cpp_deduction_guides >= 201703L
template<class E>
unexpected(E) -> unexpected<E>;
#endif

namespace detail {

template<class T>
struct is_unexpected : false_type {};
template<class E>
struct is_unexpected<unexpected<E>> : true_type {};

template<class T>
struct is_expected : false_type {};
template<class T, class E>
struct is_expected<expected<T, E>> : true_type {};

// is_constructible from every cvref of W (the is_convertible half of converts_from_any_cvref is
// intentionally omitted: [expected.object.cons] checks only is_constructible on the unexpected side).
template<class X, class W>
struct constructs_from_any_cvref {
  static constexpr bool value =
      disjunction<std::is_constructible<X, W&>, std::is_constructible<X, W const&>, std::is_constructible<X, W&&>, std::is_constructible<X, W const&&>>::value;
};

// transient "no active member" marker for the storage's default-constructed state
struct expected_uninit {};

enum class expected_state { valueless, has_value, has_error };

// [expected] reinit-expected strategy: how to construct the new arm without leaving the object
// observably valueless, chosen by the constructibility of the new arm.
enum class reinit_strategy {
  destroy_then_construct,  // new arm is nothrow-constructible from args: destroy old, construct new in place
  temporary_then_move,     // new arm is nothrow-move-constructible: build a temp, destroy old, move it in
  backup_then_restore,     // neither: back up old arm, try constructing new, restore old on throw
};

//
// expected<T, E> storage
//

template<class T, class E, bool = std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value>
struct expected_destruct_base;

template<class T, class E>
struct expected_destruct_base<T, E, true> {  // both arms trivially destructible
  union {
    expected_uninit uninit_;
    T val_;
    E unex_;
  };
  expected_state state_;

  constexpr expected_destruct_base() noexcept : uninit_(), state_(expected_state::valueless) {}

  template<class... Args>
  constexpr explicit expected_destruct_base(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : val_(std::forward<Args>(args)...), state_(expected_state::has_value)
  {
  }

  template<class... Args>
  constexpr explicit expected_destruct_base(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : unex_(std::forward<Args>(args)...), state_(expected_state::has_error)
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR void destroy() noexcept { state_ = expected_state::valueless; }
};

template<class T, class E>
struct expected_destruct_base<T, E, false> {  // at least one arm NOT trivially destructible
  union {
    expected_uninit uninit_;
    T val_;
    E unex_;
  };
  expected_state state_;

  constexpr expected_destruct_base() noexcept : uninit_(), state_(expected_state::valueless) {}

  template<class... Args>
  constexpr explicit expected_destruct_base(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : val_(std::forward<Args>(args)...), state_(expected_state::has_value)
  {
  }

  template<class... Args>
  constexpr explicit expected_destruct_base(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : unex_(std::forward<Args>(args)...), state_(expected_state::has_error)
  {
  }

  YK_POLYFILL_CXX20_CONSTEXPR ~expected_destruct_base() noexcept { destroy(); }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy() noexcept
  {
    if (state_ == expected_state::has_value) {
      val_.~T();
    } else if (state_ == expected_state::has_error) {
      unex_.~E();
    }
    state_ = expected_state::valueless;
  }
};

template<class T, class E>
struct expected_storage_base;

// [expected] reinit-expected, dispatched on reinit_strategy: construct *newp (the arm switched to)
// from args..., destroying *oldp, leaving self.state_ consistent and never observably valueless.
template<reinit_strategy S>
struct reinit_expected_impl;

template<>
struct reinit_expected_impl<reinit_strategy::destroy_then_construct> {
  template<class T, class E, class New, class Old, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(expected_storage_base<T, E>& self, New* newp, Old* oldp, expected_state new_state, expected_state,
                                                Args&&... args) noexcept
  {
    oldp->~Old();
    self.state_ = expected_state::valueless;
    polyfill::construct_at(newp, std::forward<Args>(args)...);
    self.state_ = new_state;
  }
};

template<>
struct reinit_expected_impl<reinit_strategy::temporary_then_move> {
  template<class T, class E, class New, class Old, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(expected_storage_base<T, E>& self, New* newp, Old* oldp, expected_state new_state, expected_state,
                                                Args&&... args)
  {
    New tmp(std::forward<Args>(args)...);
    oldp->~Old();
    self.state_ = expected_state::valueless;
    polyfill::construct_at(newp, std::move(tmp));
    self.state_ = new_state;
  }
};

template<>
struct reinit_expected_impl<reinit_strategy::backup_then_restore> {
  template<class T, class E, class New, class Old, class... Args>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(expected_storage_base<T, E>& self, New* newp, Old* oldp, expected_state new_state, expected_state old_state,
                                                Args&&... args)
  {
    Old tmp(std::move(*oldp));
    oldp->~Old();
    self.state_ = expected_state::valueless;
    try {
      polyfill::construct_at(newp, std::forward<Args>(args)...);
      self.state_ = new_state;
    } catch (...) {
      polyfill::construct_at(oldp, std::move(tmp));
      self.state_ = old_state;
      throw;
    }
  }
};

template<class T, class E>
struct expected_storage_base : expected_destruct_base<T, E> {
  using base = expected_destruct_base<T, E>;
  using base::base;

  YK_POLYFILL_NODISCARD constexpr bool has_value() const noexcept { return base::state_ == expected_state::has_value; }

  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T& get_value() & noexcept { return base::val_; }
  YK_POLYFILL_NODISCARD constexpr T const& get_value() const& noexcept { return base::val_; }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T&& get_value() && noexcept { return std::move(base::val_); }
  YK_POLYFILL_NODISCARD constexpr T const&& get_value() const&& noexcept { return std::move(base::val_); }

  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E& get_error() & noexcept { return base::unex_; }
  YK_POLYFILL_NODISCARD constexpr E const& get_error() const& noexcept { return base::unex_; }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E&& get_error() && noexcept { return std::move(base::unex_); }
  YK_POLYFILL_NODISCARD constexpr E const&& get_error() const&& noexcept { return std::move(base::unex_); }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_value(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
  {
    polyfill::construct_at(std::addressof(base::val_), std::forward<Args>(args)...);
    base::state_ = expected_state::has_value;
  }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_error(Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
  {
    polyfill::construct_at(std::addressof(base::unex_), std::forward<Args>(args)...);
    base::state_ = expected_state::has_error;
  }

  // [expected] reinit-expected: switch the active arm, never leaving the object valueless.
  // Precondition of reinit_to_value: state_ == has_error; of reinit_to_error: state_ == has_value.
  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void reinit_to_value(Args&&... args)
  {
    reinit_expected(std::addressof(base::val_), std::addressof(base::unex_), expected_state::has_value, expected_state::has_error, std::forward<Args>(args)...);
  }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void reinit_to_error(Args&&... args)
  {
    reinit_expected(std::addressof(base::unex_), std::addressof(base::val_), expected_state::has_error, expected_state::has_value, std::forward<Args>(args)...);
  }

  // cond_trivial_smf hooks

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_construct(expected_storage_base const& other) { construct_from(other); }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_construct(expected_storage_base&& other)
      noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_constructible<E>::value)
  {
    construct_from(std::move(other));
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_assign(expected_storage_base const& other) { assign_from(other); }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_assign(expected_storage_base&& other)
      noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<E>::value
               && std::is_nothrow_move_assignable<E>::value)
  {
    assign_from(std::move(other));
  }

private:
  // Construct the active arm from other, preserving its value category (copy vs move) via Other&&.
  template<class Other>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_from(Other&& other)
  {
    if (other.has_value()) {
      construct_value(std::forward<Other>(other).val_);
    } else {
      construct_error(std::forward<Other>(other).unex_);
    }
  }

  // Assign both arms from other, preserving its value category (copy vs move) via Other&&.
  template<class Other>
  YK_POLYFILL_CXX20_CONSTEXPR void assign_from(Other&& other)
  {
    if (has_value() && other.has_value()) {
      base::val_ = std::forward<Other>(other).val_;
    } else if (!has_value() && !other.has_value()) {
      base::unex_ = std::forward<Other>(other).unex_;
    } else if (has_value() && !other.has_value()) {
      reinit_to_error(std::forward<Other>(other).unex_);
    } else {
      reinit_to_value(std::forward<Other>(other).val_);
    }
  }

  // Construct *newp (the arm switched to) from args..., destroying *oldp; on throw, leave *oldp intact.
  template<class New, class Old, class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void reinit_expected(New* newp, Old* oldp, expected_state new_state, expected_state old_state, Args&&... args)
  {
    constexpr reinit_strategy strategy = std::is_nothrow_constructible<New, Args...>::value ? reinit_strategy::destroy_then_construct
                                         : std::is_nothrow_move_constructible<New>::value   ? reinit_strategy::temporary_then_move
                                                                                            : reinit_strategy::backup_then_restore;
    reinit_expected_impl<strategy>::apply(*this, newp, oldp, new_state, old_state, std::forward<Args>(args)...);
  }
};

//
// expected<void, E> storage
//

template<class E, bool = std::is_trivially_destructible<E>::value>
struct expected_void_destruct_base;

template<class E>
struct expected_void_destruct_base<E, true> {  // E trivially destructible
  union {
    expected_uninit uninit_;
    E unex_;
  };
  bool has_val_;

  constexpr expected_void_destruct_base() noexcept : uninit_(), has_val_(true) {}

  constexpr explicit expected_void_destruct_base(in_place_t) noexcept : uninit_(), has_val_(true) {}

  template<class... Args>
  constexpr explicit expected_void_destruct_base(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : unex_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  YK_POLYFILL_CXX14_CONSTEXPR void destroy() noexcept { has_val_ = true; }
};

template<class E>
struct expected_void_destruct_base<E, false> {  // E NOT trivially destructible
  union {
    expected_uninit uninit_;
    E unex_;
  };
  bool has_val_;

  constexpr expected_void_destruct_base() noexcept : uninit_(), has_val_(true) {}

  constexpr explicit expected_void_destruct_base(in_place_t) noexcept : uninit_(), has_val_(true) {}

  template<class... Args>
  constexpr explicit expected_void_destruct_base(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : unex_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  YK_POLYFILL_CXX20_CONSTEXPR ~expected_void_destruct_base() noexcept { destroy(); }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy() noexcept
  {
    if (!has_val_) {
      unex_.~E();
      has_val_ = true;
    }
  }
};

template<class E>
struct expected_void_storage_base : expected_void_destruct_base<E> {
  using base = expected_void_destruct_base<E>;
  using base::base;

  YK_POLYFILL_NODISCARD constexpr bool has_value() const noexcept { return base::has_val_; }

  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E& get_error() & noexcept { return base::unex_; }
  YK_POLYFILL_NODISCARD constexpr E const& get_error() const& noexcept { return base::unex_; }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E&& get_error() && noexcept { return std::move(base::unex_); }
  YK_POLYFILL_NODISCARD constexpr E const&& get_error() const&& noexcept { return std::move(base::unex_); }

  YK_POLYFILL_CXX20_CONSTEXPR void construct_valueless_value() noexcept { base::has_val_ = true; }

  template<class... Args>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_error(Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
  {
    polyfill::construct_at(std::addressof(base::unex_), std::forward<Args>(args)...);
    base::has_val_ = false;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_construct(expected_void_storage_base const& other) { construct_from(other); }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_construct(expected_void_storage_base&& other) noexcept(std::is_nothrow_move_constructible<E>::value)
  {
    construct_from(std::move(other));
  }

  YK_POLYFILL_CXX20_CONSTEXPR void _copy_assign(expected_void_storage_base const& other) { assign_from(other); }

  YK_POLYFILL_CXX20_CONSTEXPR void _move_assign(expected_void_storage_base&& other)
      noexcept(std::is_nothrow_move_constructible<E>::value && std::is_nothrow_move_assignable<E>::value)
  {
    assign_from(std::move(other));
  }

private:
  // Construct the error arm from other when it holds an error; a valued other leaves this default (valued).
  template<class Other>
  YK_POLYFILL_CXX20_CONSTEXPR void construct_from(Other&& other)
  {
    if (!other.has_value()) {
      construct_error(std::forward<Other>(other).unex_);
    }
  }

  // Assign from other, preserving its value category (copy vs move) via Other&&.
  template<class Other>
  YK_POLYFILL_CXX20_CONSTEXPR void assign_from(Other&& other)
  {
    if (!has_value() && !other.has_value()) {
      base::unex_ = std::forward<Other>(other).unex_;
    } else if (has_value() && !other.has_value()) {
      construct_error(std::forward<Other>(other).unex_);
    } else if (!has_value() && other.has_value()) {
      base::destroy();
    }
  }
};

// Extra deletion of copy/move assignment beyond what cond_trivial_smf expresses:
// [expected.object.assign] also requires (is_nothrow_move_constructible<T> || is_nothrow_move_constructible<E>).
template<bool Enable>
struct expected_assign_guard {
  expected_assign_guard() = default;
  expected_assign_guard(expected_assign_guard const&) = default;
  expected_assign_guard(expected_assign_guard&&) = default;
  expected_assign_guard& operator=(expected_assign_guard const&) = default;
  expected_assign_guard& operator=(expected_assign_guard&&) = default;
};

template<>
struct expected_assign_guard<false> {
  expected_assign_guard() = default;
  expected_assign_guard(expected_assign_guard const&) = default;
  expected_assign_guard(expected_assign_guard&&) = default;
  expected_assign_guard& operator=(expected_assign_guard const&) = delete;
  expected_assign_guard& operator=(expected_assign_guard&&) = delete;
};

// converting-constructor constraints [expected.object.cons]

template<class T, class E, class U, class G, class UF, class GF>
struct expected_can_convert {
  // [expected.object.cons] applies the converts-from-any-cvref check only "if T is not cv bool";
  // the unexpected-side check has no such carve-out.
  static constexpr bool value = std::is_constructible<T, UF>::value && std::is_constructible<E, GF>::value
                                && (std::is_same<typename std::remove_cv<T>::type, bool>::value || !converts_from_any_cvref<T, expected<U, G>>::value)
                                && !constructs_from_any_cvref<unexpected<E>, expected<U, G>>::value;
};

template<class U, class E, class G, class GF>
struct expected_void_can_convert {
  static constexpr bool value = std::is_constructible<E, GF>::value && !constructs_from_any_cvref<unexpected<E>, expected<U, G>>::value;
};

// transform helpers: build expected<U, E2> from invoking f, U possibly void

template<class Exp, class F, class... Args>
YK_POLYFILL_CXX14_CONSTEXPR Exp expected_transform_make(false_type /* U is void */, F&& f, Args&&... args)
{
  return Exp(in_place, polyfill::invoke(std::forward<F>(f), std::forward<Args>(args)...));
}

template<class Exp, class F, class... Args>
YK_POLYFILL_CXX14_CONSTEXPR Exp expected_transform_make(true_type /* U is void */, F&& f, Args&&... args)
{
  polyfill::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  return Exp(in_place);
}

}  // namespace detail

//
// expected<T, E>
//

template<class T, class E>
class expected : private detail::cond_trivial_smf<detail::expected_storage_base<T, E>, T, E>,
                 private detail::expected_assign_guard<std::is_nothrow_move_constructible<T>::value || std::is_nothrow_move_constructible<E>::value> {
private:
  using base_type = detail::cond_trivial_smf<detail::expected_storage_base<T, E>, T, E>;

  static_assert(!std::is_void<T>::value, "primary template requires a non-void T");
  static_assert(std::is_object<T>::value, "T must be an object type");
  static_assert(!std::is_array<T>::value, "T must not be an array type");
  static_assert(!std::is_reference<T>::value, "T must not be a reference type");
  static_assert(!std::is_same<typename std::remove_cv<T>::type, in_place_t>::value, "T must not be in_place_t");
  static_assert(!std::is_same<typename std::remove_cv<T>::type, unexpect_t>::value, "T must not be unexpect_t");
  static_assert(!detail::is_unexpected<typename std::remove_cv<T>::type>::value, "T must not be a specialization of unexpected");
  static_assert(std::is_object<E>::value && !std::is_array<E>::value && !std::is_const<E>::value && !std::is_volatile<E>::value,
                "E must be a valid unexpected type");

public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  template<class U>
  using rebind = expected<U, error_type>;

  // constructors

  template<class T2 = T, typename std::enable_if<std::is_default_constructible<T2>::value, std::nullptr_t>::type = nullptr>
  constexpr expected() noexcept(std::is_nothrow_default_constructible<T>::value) : base_type(in_place)
  {
  }

  // copy/move constructors are implicit (provided by cond_trivial_smf base)

  template<class U, class G,
           typename std::enable_if<detail::expected_can_convert<T, E, U, G, U const&, G const&>::value && std::is_convertible<U const&, T>::value
                                       && std::is_convertible<G const&, E>::value,
                                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected(expected<U, G> const& rhs)
      noexcept(std::is_nothrow_constructible<T, U const&>::value && std::is_nothrow_constructible<E, G const&>::value)
      : base_type()
  {
    if (rhs.has_value()) {
      this->construct_value(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  template<class U, class G,
           typename std::enable_if<detail::expected_can_convert<T, E, U, G, U const&, G const&>::value
                                       && !(std::is_convertible<U const&, T>::value && std::is_convertible<G const&, E>::value),
                                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit expected(expected<U, G> const& rhs)
      noexcept(std::is_nothrow_constructible<T, U const&>::value && std::is_nothrow_constructible<E, G const&>::value)
      : base_type()
  {
    if (rhs.has_value()) {
      this->construct_value(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  template<
      class U, class G,
      typename std::enable_if<detail::expected_can_convert<T, E, U, G, U, G>::value && std::is_convertible<U, T>::value && std::is_convertible<G, E>::value,
                              std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected(expected<U, G>&& rhs) noexcept(std::is_nothrow_constructible<T, U>::value && std::is_nothrow_constructible<E, G>::value)
      : base_type()
  {
    if (rhs.has_value()) {
      this->construct_value(*std::move(rhs));
    } else {
      this->construct_error(std::move(rhs).error());
    }
  }

  template<
      class U, class G,
      typename std::enable_if<detail::expected_can_convert<T, E, U, G, U, G>::value && !(std::is_convertible<U, T>::value && std::is_convertible<G, E>::value),
                              std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit expected(expected<U, G>&& rhs)
      noexcept(std::is_nothrow_constructible<T, U>::value && std::is_nothrow_constructible<E, G>::value)
      : base_type()
  {
    if (rhs.has_value()) {
      this->construct_value(*std::move(rhs));
    } else {
      this->construct_error(std::move(rhs).error());
    }
  }

  template<class U = typename std::remove_cv<T>::type, typename std::enable_if<
                            !std::is_same<typename remove_cvref<U>::type, in_place_t>::value && !std::is_same<typename remove_cvref<U>::type, expected>::value
                                && !detail::is_unexpected<typename remove_cvref<U>::type>::value && std::is_constructible<T, U>::value
                                && (!std::is_same<typename std::remove_cv<T>::type, bool>::value || !detail::is_expected<typename remove_cvref<U>::type>::value)
                                && std::is_convertible<U, T>::value,
                            std::nullptr_t>::type = nullptr>
  constexpr expected(U&& v) noexcept(std::is_nothrow_constructible<T, U>::value) : base_type(in_place, std::forward<U>(v))
  {
  }

  template<class U = typename std::remove_cv<T>::type, typename std::enable_if<
                            !std::is_same<typename remove_cvref<U>::type, in_place_t>::value && !std::is_same<typename remove_cvref<U>::type, expected>::value
                                && !detail::is_unexpected<typename remove_cvref<U>::type>::value && std::is_constructible<T, U>::value
                                && (!std::is_same<typename std::remove_cv<T>::type, bool>::value || !detail::is_expected<typename remove_cvref<U>::type>::value)
                                && !std::is_convertible<U, T>::value,
                            std::nullptr_t>::type = nullptr>
  constexpr explicit expected(U&& v) noexcept(std::is_nothrow_constructible<T, U>::value) : base_type(in_place, std::forward<U>(v))
  {
  }

  template<class G,
           typename std::enable_if<std::is_constructible<E, G const&>::value && std::is_convertible<G const&, E>::value, std::nullptr_t>::type = nullptr>
  constexpr expected(unexpected<G> const& e) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type(unexpect, e.error())
  {
  }

  template<class G,
           typename std::enable_if<std::is_constructible<E, G const&>::value && !std::is_convertible<G const&, E>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpected<G> const& e) noexcept(std::is_nothrow_constructible<E, G const&>::value) : base_type(unexpect, e.error())
  {
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && std::is_convertible<G, E>::value, std::nullptr_t>::type = nullptr>
  constexpr expected(unexpected<G>&& e) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type(unexpect, std::move(e).error())
  {
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && !std::is_convertible<G, E>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpected<G>&& e) noexcept(std::is_nothrow_constructible<E, G>::value) : base_type(unexpect, std::move(e).error())
  {
  }

  template<class... Args, typename std::enable_if<std::is_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
      : base_type(in_place, std::forward<Args>(args)...)
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(in_place_t, std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value)
      : base_type(in_place, il, std::forward<Args>(args)...)
  {
  }

  template<class... Args, typename std::enable_if<std::is_constructible<E, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpect_t, Args&&... args) noexcept(std::is_nothrow_constructible<E, Args...>::value)
      : base_type(unexpect, std::forward<Args>(args)...)
  {
  }

  template<class U, class... Args, typename std::enable_if<std::is_constructible<E, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible<E, std::initializer_list<U>&, Args...>::value)
      : base_type(unexpect, il, std::forward<Args>(args)...)
  {
  }

  // assignment (copy/move assignment are implicit)

  template<class U = typename std::remove_cv<T>::type, typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, expected>::value
                                                    && !detail::is_unexpected<typename remove_cvref<U>::type>::value && std::is_constructible<T, U>::value
                                                    && std::is_assignable<T&, U>::value
                                                    && (std::is_nothrow_constructible<T, U>::value || std::is_nothrow_move_constructible<T>::value
                                                        || std::is_nothrow_move_constructible<E>::value),
                                                std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected& operator=(U&& v)
  {
    if (has_value()) {
      this->get_value() = std::forward<U>(v);
    } else {
      this->reinit_to_value(std::forward<U>(v));
    }
    return *this;
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G const&>::value && std::is_assignable<E&, G const&>::value
                                                && (std::is_nothrow_constructible<E, G const&>::value || std::is_nothrow_move_constructible<T>::value
                                                    || std::is_nothrow_move_constructible<E>::value),
                                            std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected& operator=(unexpected<G> const& e)
  {
    if (has_value()) {
      this->reinit_to_error(e.error());
    } else {
      this->get_error() = e.error();
    }
    return *this;
  }

  template<class G, typename std::enable_if<std::is_constructible<E, G>::value && std::is_assignable<E&, G>::value
                                                && (std::is_nothrow_constructible<E, G>::value || std::is_nothrow_move_constructible<T>::value
                                                    || std::is_nothrow_move_constructible<E>::value),
                                            std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR expected& operator=(unexpected<G>&& e)
  {
    if (has_value()) {
      this->reinit_to_error(std::move(e).error());
    } else {
      this->get_error() = std::move(e).error();
    }
    return *this;
  }

  template<class... Args, typename std::enable_if<std::is_nothrow_constructible<T, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(Args&&... args) noexcept
  {
    this->destroy();
    this->construct_value(std::forward<Args>(args)...);
    return this->get_value();
  }

  template<class U, class... Args,
           typename std::enable_if<std::is_nothrow_constructible<T, std::initializer_list<U>&, Args...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR T& emplace(std::initializer_list<U> il, Args&&... args) noexcept
  {
    this->destroy();
    this->construct_value(il, std::forward<Args>(args)...);
    return this->get_value();
  }

  // swap

  // [expected.object.swap]
  template<class T2 = T, class E2 = E,
           typename std::enable_if<is_swappable<T2>::value && is_swappable<E2>::value && std::is_move_constructible<T2>::value
                                       && std::is_move_constructible<E2>::value
                                       && (std::is_nothrow_move_constructible<T2>::value || std::is_nothrow_move_constructible<E2>::value),
                                   std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible<T>::value && is_nothrow_swappable<T>::value
                                                                && std::is_nothrow_move_constructible<E>::value && is_nothrow_swappable<E>::value)
  {
    if (rhs.has_value()) {
      if (has_value()) {
        using std::swap;
        swap(this->get_value(), rhs.get_value());
      } else {
        rhs.swap(*this);
      }
    } else {
      if (has_value()) {
        swap_value_error(rhs, integral_constant<bool, std::is_nothrow_move_constructible<E>::value>{});
      } else {
        using std::swap;
        swap(this->get_error(), rhs.get_error());
      }
    }
  }

  // observers

  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T const* operator->() const noexcept { return std::addressof(this->get_value()); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T* operator->() noexcept { return std::addressof(this->get_value()); }

  YK_POLYFILL_NODISCARD constexpr T const& operator*() const& noexcept { return this->get_value(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T& operator*() & noexcept { return this->get_value(); }
  YK_POLYFILL_NODISCARD constexpr T const&& operator*() const&& noexcept { return std::move(*this).base_get_value(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR T&& operator*() && noexcept { return std::move(*this).base_get_value(); }

  YK_POLYFILL_NODISCARD constexpr explicit operator bool() const noexcept { return has_value(); }
  YK_POLYFILL_NODISCARD constexpr bool has_value() const noexcept { return base_type::has_value(); }

  YK_POLYFILL_CXX14_CONSTEXPR T& value() &
  {
    static_assert(std::is_copy_constructible<E>::value, "E must be copy constructible");
    if (has_value()) return this->get_value();
    throw bad_expected_access<E>(this->get_error());
  }
  YK_POLYFILL_CXX14_CONSTEXPR T const& value() const&
  {
    static_assert(std::is_copy_constructible<E>::value, "E must be copy constructible");
    if (has_value()) return this->get_value();
    throw bad_expected_access<E>(this->get_error());
  }
  YK_POLYFILL_CXX14_CONSTEXPR T&& value() &&
  {
    static_assert(std::is_copy_constructible<E>::value && std::is_constructible<E, E&&>::value, "E must be copy constructible and constructible from E&&");
    if (has_value()) return std::move(this->get_value());
    throw bad_expected_access<E>(std::move(this->get_error()));
  }
  YK_POLYFILL_CXX14_CONSTEXPR T const&& value() const&&
  {
    static_assert(std::is_copy_constructible<E>::value && std::is_constructible<E, E const&&>::value,
                  "E must be copy constructible and constructible from E const&&");
    if (has_value()) return std::move(this->get_value());
    throw bad_expected_access<E>(std::move(this->get_error()));
  }

  YK_POLYFILL_NODISCARD constexpr E const& error() const& noexcept { return this->get_error(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E& error() & noexcept { return this->get_error(); }
  YK_POLYFILL_NODISCARD constexpr E const&& error() const&& noexcept { return std::move(*this).base_get_error(); }
  YK_POLYFILL_NODISCARD YK_POLYFILL_CXX14_CONSTEXPR E&& error() && noexcept { return std::move(*this).base_get_error(); }

  template<class U = typename std::remove_cv<T>::type>
  YK_POLYFILL_CXX14_CONSTEXPR T value_or(U&& v) const&
  {
    static_assert(std::is_copy_constructible<T>::value && std::is_convertible<U, T>::value, "T must be copy constructible and the argument convertible to T");
    return has_value() ? this->get_value() : static_cast<T>(std::forward<U>(v));
  }

  template<class U = typename std::remove_cv<T>::type>
  YK_POLYFILL_CXX14_CONSTEXPR T value_or(U&& v) &&
  {
    static_assert(std::is_move_constructible<T>::value && std::is_convertible<U, T>::value, "T must be move constructible and the argument convertible to T");
    return has_value() ? std::move(this->get_value()) : static_cast<T>(std::forward<U>(v));
  }

  template<class G = E>
  YK_POLYFILL_CXX14_CONSTEXPR E error_or(G&& e) const&
  {
    static_assert(std::is_copy_constructible<E>::value && std::is_convertible<G, E>::value, "E must be copy constructible and the argument convertible to E");
    return has_value() ? static_cast<E>(std::forward<G>(e)) : this->get_error();
  }

  template<class G = E>
  YK_POLYFILL_CXX14_CONSTEXPR E error_or(G&& e) &&
  {
    static_assert(std::is_move_constructible<E>::value && std::is_convertible<G, E>::value, "E must be move constructible and the argument convertible to E");
    return has_value() ? static_cast<E>(std::forward<G>(e)) : std::move(this->get_error());
  }

  // monadic operations

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) & -> typename remove_cvref<typename invoke_result<F, T&>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F, T&>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f), this->get_value());
    return U(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const& -> typename remove_cvref<typename invoke_result<F, T const&>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F, T const&>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f), this->get_value());
    return U(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) && -> typename remove_cvref<typename invoke_result<F, T&&>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F, T&&>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f), std::move(this->get_value()));
    return U(unexpect, std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto and_then(F&& f) const&& -> typename remove_cvref<typename invoke_result<F, T const&&>::type>::type
  {
    using U = typename remove_cvref<typename invoke_result<F, T const&&>::type>::type;
    static_assert(detail::is_expected<U>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename U::error_type, E>::value, "F's expected result must have the same error_type");
    if (has_value()) return polyfill::invoke(std::forward<F>(f), std::move(this->get_value()));
    return U(unexpect, std::move(this->get_error()));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) & -> typename remove_cvref<typename invoke_result<F, E&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, T>::value, "F's expected result must have the same value_type");
    if (has_value()) return G(in_place, this->get_value());
    return polyfill::invoke(std::forward<F>(f), this->get_error());
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) const& -> typename remove_cvref<typename invoke_result<F, E const&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E const&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, T>::value, "F's expected result must have the same value_type");
    if (has_value()) return G(in_place, this->get_value());
    return polyfill::invoke(std::forward<F>(f), this->get_error());
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) && -> typename remove_cvref<typename invoke_result<F, E&&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E&&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, T>::value, "F's expected result must have the same value_type");
    if (has_value()) return G(in_place, std::move(this->get_value()));
    return polyfill::invoke(std::forward<F>(f), std::move(this->get_error()));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto or_else(F&& f) const&& -> typename remove_cvref<typename invoke_result<F, E const&&>::type>::type
  {
    using G = typename remove_cvref<typename invoke_result<F, E const&&>::type>::type;
    static_assert(detail::is_expected<G>::value, "result of F must be a specialization of expected");
    static_assert(std::is_same<typename G::value_type, T>::value, "F's expected result must have the same value_type");
    if (has_value()) return G(in_place, std::move(this->get_value()));
    return polyfill::invoke(std::forward<F>(f), std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) & -> expected<typename std::remove_cv<typename invoke_result<F, T&>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F, T&>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f), this->get_value());
    return expected<U, E>(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const& -> expected<typename std::remove_cv<typename invoke_result<F, T const&>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F, T const&>::type>::type;
    if (has_value()) return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f), this->get_value());
    return expected<U, E>(unexpect, this->get_error());
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) && -> expected<typename std::remove_cv<typename invoke_result<F, T&&>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F, T&&>::type>::type;
    if (has_value())
      return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f), std::move(this->get_value()));
    return expected<U, E>(unexpect, std::move(this->get_error()));
  }

  template<class F, class E2 = E, typename std::enable_if<std::is_constructible<E2, E2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform(F&& f) const&& -> expected<typename std::remove_cv<typename invoke_result<F, T const&&>::type>::type, E>
  {
    using U = typename std::remove_cv<typename invoke_result<F, T const&&>::type>::type;
    if (has_value())
      return detail::expected_transform_make<expected<U, E>>(bool_constant<std::is_void<U>::value>{}, std::forward<F>(f), std::move(this->get_value()));
    return expected<U, E>(unexpect, std::move(this->get_error()));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) & -> expected<T, typename std::remove_cv<typename invoke_result<F, E&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E&>::type>::type;
    if (has_value()) return expected<T, G>(in_place, this->get_value());
    return expected<T, G>(unexpect, polyfill::invoke(std::forward<F>(f), this->get_error()));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2 const&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) const& -> expected<T, typename std::remove_cv<typename invoke_result<F, E const&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E const&>::type>::type;
    if (has_value()) return expected<T, G>(in_place, this->get_value());
    return expected<T, G>(unexpect, polyfill::invoke(std::forward<F>(f), this->get_error()));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) && -> expected<T, typename std::remove_cv<typename invoke_result<F, E&&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E&&>::type>::type;
    if (has_value()) return expected<T, G>(in_place, std::move(this->get_value()));
    return expected<T, G>(unexpect, polyfill::invoke(std::forward<F>(f), std::move(this->get_error())));
  }

  template<class F, class T2 = T, typename std::enable_if<std::is_constructible<T2, T2 const&&>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR auto transform_error(F&& f) const&& -> expected<T, typename std::remove_cv<typename invoke_result<F, E const&&>::type>::type>
  {
    using G = typename std::remove_cv<typename invoke_result<F, E const&&>::type>::type;
    if (has_value()) return expected<T, G>(in_place, std::move(this->get_value()));
    return expected<T, G>(unexpect, polyfill::invoke(std::forward<F>(f), std::move(this->get_error())));
  }

private:
  // rvalue accessors on the storage (private base), reached through explicit std::move.
  YK_POLYFILL_CXX14_CONSTEXPR T&& base_get_value() && noexcept { return static_cast<base_type&&>(*this).get_value(); }
  constexpr T const&& base_get_value() const&& noexcept { return static_cast<base_type const&&>(*this).get_value(); }
  YK_POLYFILL_CXX14_CONSTEXPR E&& base_get_error() && noexcept { return static_cast<base_type&&>(*this).get_error(); }
  constexpr E const&& base_get_error() const&& noexcept { return static_cast<base_type const&&>(*this).get_error(); }

  // this holds a value, rhs holds an error; leave this holding rhs's error and rhs holding this's value.
  YK_POLYFILL_CXX20_CONSTEXPR void swap_value_error(expected& rhs, true_type /* E is nothrow move constructible */)
  {
    E tmp(std::move(rhs.get_error()));
    rhs.destroy();
    try {
      rhs.construct_value(std::move(this->get_value()));
      this->destroy();
      this->construct_error(std::move(tmp));
    } catch (...) {
      rhs.construct_error(std::move(tmp));
      throw;
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void swap_value_error(expected& rhs, false_type /* rely on T nothrow move constructible */)
  {
    T tmp(std::move(this->get_value()));
    this->destroy();
    try {
      this->construct_error(std::move(rhs.get_error()));
      rhs.destroy();
      rhs.construct_value(std::move(tmp));
    } catch (...) {
      this->construct_value(std::move(tmp));
      throw;
    }
  }
};

//
// expected<cv void, E> [expected.void]
//
// The standard defines one partial specialization constrained by is_void_v<T>; without C++20
// constraints, stamp it out once per cv-qualification of void via expected_void.ipp.

}  // namespace polyfill

}  // namespace yk

#define YK_POLYFILL_INCLUDE_EXPECTED

#define YK_POLYFILL_EXPECTED_VOID_CV
#include <yk/polyfill/bits/expected_void.ipp>
#undef YK_POLYFILL_EXPECTED_VOID_CV

#define YK_POLYFILL_EXPECTED_VOID_CV const
#include <yk/polyfill/bits/expected_void.ipp>
#undef YK_POLYFILL_EXPECTED_VOID_CV

#define YK_POLYFILL_EXPECTED_VOID_CV volatile
#include <yk/polyfill/bits/expected_void.ipp>
#undef YK_POLYFILL_EXPECTED_VOID_CV

#define YK_POLYFILL_EXPECTED_VOID_CV const volatile
#include <yk/polyfill/bits/expected_void.ipp>
#undef YK_POLYFILL_EXPECTED_VOID_CV

#undef YK_POLYFILL_INCLUDE_EXPECTED

namespace yk {

namespace polyfill {

// comparisons
//
// Workaround: SFINAE via return type instead of default template parameter, matching
// variant. MSVC fails to deduce a default enable_if parameter for C++20 reversed
// comparison candidates. In C++20 only operator== is defined (the language rewrites !=,
// and synthesizes the reversed value==expected / unexpected==expected candidates).

template<class T1, class E1, class T2, class E2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !std::is_void<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value
                                                        && std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value,
                                                    bool>::type
operator==(expected<T1, E1> const& lhs, expected<T2, E2> const& rhs)
{
  if (lhs.has_value() != rhs.has_value()) return false;
  if (lhs.has_value()) return static_cast<bool>(*lhs == *rhs);
  return static_cast<bool>(lhs.error() == rhs.error());
}

template<class T1, class E1, class T2, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_void<T1>::value && std::is_void<T2>::value
                                && std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value,
                            bool>::type
    operator==(expected<T1, E1> const& lhs, expected<T2, E2> const& rhs)
{
  if (lhs.has_value() != rhs.has_value()) return false;
  if (lhs.has_value()) return true;
  return static_cast<bool>(lhs.error() == rhs.error());
}

template<class T1, class E1, class T2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !detail::is_expected<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value,
                                                    bool>::type
operator==(expected<T1, E1> const& lhs, T2 const& rhs)
{
  return lhs.has_value() && static_cast<bool>(*lhs == rhs);
}

template<class T1, class E1, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value, bool>::type
    operator==(expected<T1, E1> const& lhs, unexpected<E2> const& rhs)
{
  return !lhs.has_value() && static_cast<bool>(lhs.error() == rhs.error());
}

#if __cplusplus < 202002L

template<class T1, class E1, class T2, class E2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !std::is_void<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value
                                                        && std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value,
                                                    bool>::type operator!=(expected<T1, E1> const& lhs, expected<T2, E2> const& rhs)
{
  return !(lhs == rhs);
}

template<class T1, class E1, class T2, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_void<T1>::value && std::is_void<T2>::value
                                && std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value,
                            bool>::type
    operator!=(expected<T1, E1> const& lhs, expected<T2, E2> const& rhs)
{
  return !(lhs == rhs);
}

template<class T1, class E1, class T2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !detail::is_expected<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value,
                                                    bool>::type
operator==(T2 const& lhs, expected<T1, E1> const& rhs)
{
  return rhs.has_value() && static_cast<bool>(*rhs == lhs);
}

template<class T1, class E1, class T2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !detail::is_expected<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value,
                                                    bool>::type
operator!=(expected<T1, E1> const& lhs, T2 const& rhs)
{
  return !(lhs == rhs);
}

template<class T1, class E1, class T2>
YK_POLYFILL_CXX14_CONSTEXPR typename std::enable_if<!std::is_void<T1>::value && !detail::is_expected<T2>::value
                                                        && std::is_convertible<decltype(std::declval<T1 const&>() == std::declval<T2 const&>()), bool>::value,
                                                    bool>::type
operator!=(T2 const& lhs, expected<T1, E1> const& rhs)
{
  return !(rhs == lhs);
}

template<class T1, class E1, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value, bool>::type
    operator==(unexpected<E2> const& lhs, expected<T1, E1> const& rhs)
{
  return !rhs.has_value() && static_cast<bool>(rhs.error() == lhs.error());
}

template<class T1, class E1, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value, bool>::type
    operator!=(expected<T1, E1> const& lhs, unexpected<E2> const& rhs)
{
  return !(lhs == rhs);
}

template<class T1, class E1, class E2>
YK_POLYFILL_CXX14_CONSTEXPR
    typename std::enable_if<std::is_convertible<decltype(std::declval<E1 const&>() == std::declval<E2 const&>()), bool>::value, bool>::type
    operator!=(unexpected<E2> const& lhs, expected<T1, E1> const& rhs)
{
  return !(rhs == lhs);
}

#endif

// Mirrors the member swap constraints: [expected.void.swap] for void T (E alone),
// [expected.object.swap] otherwise (both arms, plus the nothrow-move clause).
template<class T, class E,
         typename std::enable_if<is_swappable<E>::value && std::is_move_constructible<E>::value
                                     && (std::is_void<T>::value
                                         || (is_swappable<T>::value && std::is_move_constructible<T>::value
                                             && (std::is_nothrow_move_constructible<T>::value || std::is_nothrow_move_constructible<E>::value))),
                                 std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX20_CONSTEXPR void swap(expected<T, E>& lhs, expected<T, E>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_EXPECTED_HPP
