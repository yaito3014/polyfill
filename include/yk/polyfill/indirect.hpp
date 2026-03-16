#ifndef YK_POLYFILL_INDIRECT_HPP
#define YK_POLYFILL_INDIRECT_HPP

// indirect<T, A>: value-semantic wrapper for a heap-allocated T
// Polyfill of P3019R14 std::indirect for C++11 and later.

#include <yk/polyfill/config.hpp>
#include <yk/polyfill/bits/allocator_is_always_equal.hpp>
#include <yk/polyfill/bits/swap.hpp>
#include <yk/polyfill/extension/ebo_storage.hpp>
#include <yk/polyfill/utility.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

// Forward declaration so the ops structs can reference indirect
template<class T, class A>
class indirect;

namespace detail {

template<class T>
struct is_indirect : std::false_type {};

template<class T, class A>
struct is_indirect<indirect<T, A>> : std::true_type {};

// synth_three_way: like <=> but falls back to synthesising weak_ordering from
// < and == when the type has no <=> (mirrors the standard's synth-three-way).
#if __cpp_lib_three_way_comparison >= 201907L
template<class T, class U = T>
constexpr auto synth_three_way(T const& t, U const& u)
  requires requires {
    { t < u } -> std::convertible_to<bool>;
    { u < t } -> std::convertible_to<bool>;
  }
{
  if constexpr (std::three_way_comparable_with<T, U>) {
    return t <=> u;
  } else {
    if (t < u) return std::weak_ordering::less;
    if (u < t) return std::weak_ordering::greater;
    return std::weak_ordering::equivalent;
  }
}

template<class T, class U = T>
using synth_three_way_result = decltype(synth_three_way(std::declval<T const&>(), std::declval<U const&>()));
#endif  // __cpp_lib_three_way_comparison

template<bool Pocs>
struct indirect_swap_ops;
template<bool Pocca>
struct indirect_copy_assign_ops;
template<bool Pocma>
struct indirect_move_assign_ops;
template<bool AlwaysEqual>
struct indirect_move_assign_ne_ops;  // POCMA=false path
template<bool AlwaysEqual>
struct indirect_move_ctor_ops;  // extended-alloc move ctor

}  // namespace detail

template<class T, class A = std::allocator<T>>
class indirect : private extension::ebo_storage<A> {
  static_assert(!std::is_array<T>::value, "indirect: T must not be an array type");

  using alloc_base = extension::ebo_storage<A>;
  using alloc_traits = std::allocator_traits<A>;

  T* ptr_;

  // --- Private helpers (called by ops specialisations via friendship) --------

  template<class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR void allocate_and_construct(Ts&&... ts)
  {
    ptr_ = alloc_traits::allocate(this->stored_value(), 1);
    try {
      alloc_traits::construct(this->stored_value(), ptr_, static_cast<Ts&&>(ts)...);
    } catch (...) {
      alloc_traits::deallocate(this->stored_value(), ptr_, 1);
      ptr_ = nullptr;
      throw;
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy_owned() noexcept
  {
    if (ptr_ != nullptr) {
      alloc_traits::destroy(this->stored_value(), ptr_);
      alloc_traits::deallocate(this->stored_value(), ptr_, 1);
      ptr_ = nullptr;
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void copy_assign_content(indirect const& other)
  {
    if (other.ptr_ == nullptr) {
      destroy_owned();
    } else if (ptr_ != nullptr) {
      *ptr_ = *other.ptr_;
    } else {
      allocate_and_construct(*other.ptr_);
    }
  }

  // --- Friends: ops specialisations need access to private members -----------

  template<bool>
  friend struct detail::indirect_swap_ops;
  template<bool>
  friend struct detail::indirect_copy_assign_ops;
  template<bool>
  friend struct detail::indirect_move_assign_ops;
  template<bool>
  friend struct detail::indirect_move_assign_ne_ops;
  template<bool>
  friend struct detail::indirect_move_ctor_ops;

public:
  using value_type = T;
  using allocator_type = A;

  // --- Constructors ---

  // Constraint: A must be default-constructible (enable_if)
  // Mandate:    T must be default-constructible (static_assert)
  template<typename AllocDummy = A, typename std::enable_if<std::is_default_constructible<AllocDummy>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR indirect() : alloc_base(), ptr_(nullptr)
  {
    static_assert(std::is_default_constructible<T>::value, "indirect: T must be default-constructible");
    allocate_and_construct();
  }

  // Mandate: T must be default-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, A const& a) : alloc_base(a), ptr_(nullptr)
  {
    static_assert(std::is_default_constructible<T>::value, "indirect: T must be default-constructible");
    allocate_and_construct();
  }

  // Constraints: U not indirect, U not in_place_t, T constructible from U, A default-constructible (enable_if)
  template<
      class U = T, typename AllocDummy = A,
      typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, indirect>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, in_place_t>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_default_constructible<AllocDummy>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(U&& u) : alloc_base(), ptr_(nullptr)
  {
    allocate_and_construct(static_cast<U&&>(u));
  }

  // Constraints: U not indirect, U not in_place_t, T constructible from U (enable_if)
  template<
      class U = T, typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, indirect>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, in_place_t>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, A const& a, U&& u) : alloc_base(a), ptr_(nullptr)
  {
    allocate_and_construct(static_cast<U&&>(u));
  }

  // Constraints: T constructible from Ts...; A must be default-constructible (enable_if)
  template<
      class... Ts, typename AllocDummy2 = A, typename std::enable_if<std::is_constructible<T, Ts...>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_default_constructible<AllocDummy2>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(in_place_t, Ts&&... ts) : alloc_base(), ptr_(nullptr)
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  // Constraint: T constructible from Ts... (enable_if)
  template<class... Ts, typename std::enable_if<std::is_constructible<T, Ts...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, A const& a, in_place_t, Ts&&... ts) : alloc_base(a), ptr_(nullptr)
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  // Mandate: T must be copy-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect const& other)
      : alloc_base(alloc_traits::select_on_container_copy_construction(other.stored_value())), ptr_(nullptr)
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  // Mandate: T must be copy-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect const& other, std::allocator_arg_t, A const& a) : alloc_base(a), ptr_(nullptr)
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  // (no constraint on T: move always works)
  YK_POLYFILL_CXX14_CONSTEXPR indirect(indirect&& other) noexcept : alloc_base(static_cast<A&&>(other.stored_value())), ptr_(other.ptr_)
  {
    other.ptr_ = nullptr;
  }

  // Mandate: T must be move-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect&& other, std::allocator_arg_t, A const& a) noexcept(detail::allocator_is_always_equal<A>::value)
      : alloc_base(a), ptr_(nullptr)
  {
    static_assert(std::is_move_constructible<T>::value, "indirect: T must be move-constructible");
    detail::indirect_move_ctor_ops<detail::allocator_is_always_equal<A>::value>::apply(*this, static_cast<indirect&&>(other));
  }

  // --- Destructor ---

  YK_POLYFILL_CXX20_CONSTEXPR ~indirect() noexcept { destroy_owned(); }

  // --- Assignment ---

  // Mandates: T must be copy-constructible and copy-assignable (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(indirect const& other)
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    static_assert(std::is_copy_assignable<T>::value, "indirect: T must be copy-assignable");
    if (this == &other) return *this;
    detail::indirect_copy_assign_ops<alloc_traits::propagate_on_container_copy_assignment::value>::apply(*this, other);
    return *this;
  }

  // Mandate: T must be move-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(indirect&& other) noexcept(
      alloc_traits::propagate_on_container_move_assignment::value || detail::allocator_is_always_equal<A>::value
  )
  {
    static_assert(std::is_move_constructible<T>::value, "indirect: T must be move-constructible");
    if (this == &other) return *this;
    detail::indirect_move_assign_ops<alloc_traits::propagate_on_container_move_assignment::value>::apply(*this, static_cast<indirect&&>(other));
    return *this;
  }

  // Constraints: U not indirect; T constructible from U and assignable from U
  template<
      class U, typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, indirect>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr,
      typename std::enable_if<std::is_assignable<T&, U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(U&& u)
  {
    if (ptr_ != nullptr) {
      *ptr_ = static_cast<U&&>(u);
    } else {
      allocate_and_construct(static_cast<U&&>(u));
    }
    return *this;
  }

  // --- Observers ---

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T& operator*() & noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T& operator*() const& noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T&& operator*() && noexcept { return static_cast<T&&>(*ptr_); }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T&& operator*() const&& noexcept { return static_cast<T const&&>(*ptr_); }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T* operator->() noexcept { return ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T* operator->() const noexcept { return ptr_; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR bool valueless_after_move() const noexcept { return ptr_ == nullptr; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR A get_allocator() const noexcept { return this->stored_value(); }

  // --- Swap ---

  YK_POLYFILL_CXX14_CONSTEXPR void swap(indirect& other) noexcept(
      alloc_traits::propagate_on_container_swap::value || detail::allocator_is_always_equal<A>::value
  )
  {
    detail::indirect_swap_ops<alloc_traits::propagate_on_container_swap::value>::apply(*this, other);
  }

  friend YK_POLYFILL_CXX14_CONSTEXPR void swap(indirect& a, indirect& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

  // --- Comparison ---

  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator==(indirect const& lhs, indirect const& rhs) noexcept(
      noexcept(std::declval<T const&>() == std::declval<T const&>())
  )
  {
    if (lhs.valueless_after_move()) return rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return false;
    return *lhs == *rhs;
  }

  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(indirect const& lhs, indirect const& rhs) noexcept(
      noexcept(std::declval<T const&>() != std::declval<T const&>())
  )
  {
    if (lhs.valueless_after_move()) return !rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return true;
    return *lhs != *rhs;
  }

#if __cpp_lib_three_way_comparison >= 201907L
  // Handles same-type and cross-type indirect<U,AA> comparisons.
  // Must be a function template so the return type is deferred and not
  // evaluated eagerly at class instantiation (avoids hard errors for T
  // types that lack operator< / operator<=>).
  template<class U, class AA>
  friend constexpr auto operator<=>(indirect<T, A> const& lhs, indirect<U, AA> const& rhs) -> detail::synth_three_way_result<T, U>
  {
    if (lhs.valueless_after_move() || rhs.valueless_after_move()) {
      return !lhs.valueless_after_move() <=> !rhs.valueless_after_move();
    }
    return detail::synth_three_way(*lhs, *rhs);
  }

  template<class U>
  friend constexpr auto operator<=>(indirect<T, A> const& lhs, U const& rhs) -> detail::synth_three_way_result<T, U>
    requires (!detail::is_indirect<U>::value)
  {
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    return detail::synth_three_way(*lhs, rhs);
  }
#endif  // __cpp_lib_three_way_comparison
};

// ---- Heterogeneous comparisons (outside class to avoid MSVC ADL recursion) ----

template<class T, class A, class U, typename std::enable_if<!detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator==(indirect<T, A> const& lhs, U const& rhs) noexcept(noexcept(std::declval<T const&>() == std::declval<U const&>()))
{
  if (lhs.valueless_after_move()) return false;
  return *lhs == rhs;
}

template<class T, class A, class U, typename std::enable_if<!detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator==(U const& lhs, indirect<T, A> const& rhs) noexcept(noexcept(std::declval<U const&>() == std::declval<T const&>()))
{
  if (rhs.valueless_after_move()) return false;
  return lhs == *rhs;
}

template<class T, class A, class U, typename std::enable_if<!detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(indirect<T, A> const& lhs, U const& rhs) noexcept(noexcept(std::declval<T const&>() != std::declval<U const&>()))
{
  if (lhs.valueless_after_move()) return true;
  return *lhs != rhs;
}

template<class T, class A, class U, typename std::enable_if<!detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(U const& lhs, indirect<T, A> const& rhs) noexcept(noexcept(std::declval<U const&>() != std::declval<T const&>()))
{
  if (rhs.valueless_after_move()) return true;
  return lhs != *rhs;
}

// ---- Cross-type wrapper comparisons (outside class to avoid MSVC ADL recursion) ----

template<
    class T, class A, class U, class AA, typename std::enable_if<!std::is_same<T, U>::value || !std::is_same<A, AA>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator==(indirect<T, A> const& lhs, indirect<U, AA> const& rhs) noexcept(
    noexcept(std::declval<T const&>() == std::declval<U const&>())
)
{
  if (lhs.valueless_after_move()) return rhs.valueless_after_move();
  if (rhs.valueless_after_move()) return false;
  return *lhs == *rhs;
}

template<
    class T, class A, class U, class AA, typename std::enable_if<!std::is_same<T, U>::value || !std::is_same<A, AA>::value, std::nullptr_t>::type = nullptr>
YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(indirect<T, A> const& lhs, indirect<U, AA> const& rhs) noexcept(
    noexcept(std::declval<T const&>() != std::declval<U const&>())
)
{
  if (lhs.valueless_after_move()) return !rhs.valueless_after_move();
  if (rhs.valueless_after_move()) return true;
  return *lhs != *rhs;
}

#if __cplusplus >= 201703L
// Deduction guides
template<class T>
indirect(T) -> indirect<T>;

template<class A, class T>
indirect(std::allocator_arg_t, A, T) -> indirect<T, typename std::allocator_traits<A>::template rebind_alloc<T>>;
#endif  // __cplusplus >= 201703L

// ---- ops specialisations (indirect is now complete) -------------------------

namespace detail {

template<>
struct indirect_swap_ops</*Pocs = */ true> {
  template<class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& a, indirect<T, A>& b) noexcept
  {
    detail::constexpr_swap(a.stored_value(), b.stored_value());
    detail::constexpr_swap(a.ptr_, b.ptr_);
  }
};

template<>
struct indirect_swap_ops</*Pocs = */ false> {
  template<class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& a, indirect<T, A>& b) noexcept
  {
    detail::constexpr_swap(a.ptr_, b.ptr_);
  }
};

template<>
struct indirect_copy_assign_ops</*Pocca = */ true> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A> const& other)
  {
    if (self.stored_value() != other.stored_value()) {
      self.destroy_owned();
      self.stored_value() = other.stored_value();
      if (other.ptr_ != nullptr) self.allocate_and_construct(*other.ptr_);
    } else {
      self.copy_assign_content(other);
    }
  }
};

template<>
struct indirect_copy_assign_ops</*Pocca = */ false> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A> const& other)
  {
    self.copy_assign_content(other);
  }
};

template<>
struct indirect_move_assign_ops</*Pocma = */ true> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.stored_value() = static_cast<A&&>(other.stored_value());
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template<>
struct indirect_move_assign_ops</*Pocma = */ false> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept(detail::allocator_is_always_equal<A>::value)
  {
    indirect_move_assign_ne_ops<detail::allocator_is_always_equal<A>::value>::apply(self, static_cast<indirect<T, A>&&>(other));
  }
};

template<>
struct indirect_move_assign_ne_ops</*AlwaysEqual = */ true> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template<>
struct indirect_move_assign_ne_ops</*AlwaysEqual = */ false> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other)
  {
    if (self.stored_value() == other.stored_value()) {
      self.destroy_owned();
      self.ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else if (other.ptr_ != nullptr) {
      if (self.ptr_ != nullptr) {
        *self.ptr_ = static_cast<T&&>(*other.ptr_);
      } else {
        self.allocate_and_construct(static_cast<T&&>(*other.ptr_));
      }
    } else {
      self.destroy_owned();
    }
  }
};

template<>
struct indirect_move_ctor_ops</*AlwaysEqual = */ true> {
  template<class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template<>
struct indirect_move_ctor_ops</*AlwaysEqual = */ false> {
  template<class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other)
  {
    if (self.stored_value() == other.stored_value()) {
      self.ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else if (other.ptr_ != nullptr) {
      self.allocate_and_construct(static_cast<T&&>(*other.ptr_));
    }
  }
};

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

// Hash specialization
namespace std {

template<class T, class A>
struct hash<yk::polyfill::indirect<T, A>> {
  std::size_t operator()(yk::polyfill::indirect<T, A> const& ind) const noexcept(noexcept(std::hash<T>{}(*ind)))
  {
    if (ind.valueless_after_move()) return std::hash<T*>{}(nullptr);
    return std::hash<T>{}(*ind);
  }
};

}  // namespace std

#endif  // YK_POLYFILL_INDIRECT_HPP
