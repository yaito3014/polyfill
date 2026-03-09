#ifndef YK_POLYFILL_INDIRECT_HPP
#define YK_POLYFILL_INDIRECT_HPP

// indirect<T, A>: value-semantic wrapper for a heap-allocated T
// Polyfill of P3019R14 std::indirect for C++11 and later.

#include <yk/polyfill/config.hpp>
#include <yk/polyfill/utility.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

// Forward declaration so the ops structs can reference indirect
template <class T, class A>
class indirect;

namespace indirect_detail {

template <class T>
struct is_indirect : std::false_type {};

template <class T, class A>
struct is_indirect<indirect<T, A>> : std::true_type {};

// Fallback for is_always_equal (added to allocator_traits in C++17)
#if __cpp_lib_allocator_traits_is_always_equal >= 201411L
template <class A>
struct is_always_equal : std::allocator_traits<A>::is_always_equal {};
#else
template <class A>
struct is_always_equal : std::is_empty<A> {};
#endif

// constexpr-friendly swap: std::swap is not constexpr before C++20
template <class T>
YK_POLYFILL_CXX14_CONSTEXPR void cswap(T& a, T& b)
    noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value)
{
  T tmp(static_cast<T&&>(a));
  a = static_cast<T&&>(b);
  b = static_cast<T&&>(tmp);
}

// Forward declarations — specialisations are defined after indirect is complete.
template <bool Pocs>        struct swap_ops;
template <bool Pocca>       struct copy_assign_ops;
template <bool Pocma>       struct move_assign_ops;
template <bool AlwaysEqual> struct move_assign_ne_ops;  // POCMA=false path
template <bool AlwaysEqual> struct move_ctor_ops;       // extended-alloc move ctor

}  // namespace indirect_detail

template <class T, class A = std::allocator<T>>
class indirect {
  static_assert(!std::is_array<T>::value, "indirect: T must not be an array type");

  using alloc_traits = std::allocator_traits<A>;

  T* ptr_;
  YK_POLYFILL_NO_UNIQUE_ADDRESS A alloc_;

  // --- Private helpers (called by ops specialisations via friendship) --------

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR void allocate_and_construct(Ts&&... ts)
  {
    ptr_ = alloc_traits::allocate(alloc_, 1);
    try {
      alloc_traits::construct(alloc_, ptr_, static_cast<Ts&&>(ts)...);
    } catch (...) {
      alloc_traits::deallocate(alloc_, ptr_, 1);
      ptr_ = nullptr;
      throw;
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy_owned() noexcept
  {
    if (ptr_ != nullptr) {
      alloc_traits::destroy(alloc_, ptr_);
      alloc_traits::deallocate(alloc_, ptr_, 1);
      ptr_ = nullptr;
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void copy_assign_content(const indirect& other)
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

  template <bool> friend struct indirect_detail::swap_ops;
  template <bool> friend struct indirect_detail::copy_assign_ops;
  template <bool> friend struct indirect_detail::move_assign_ops;
  template <bool> friend struct indirect_detail::move_assign_ne_ops;
  template <bool> friend struct indirect_detail::move_ctor_ops;

 public:
  using value_type = T;
  using allocator_type = A;

  // --- Constructors ---

  // Constraint: A must be default-constructible (enable_if)
  // Mandate:    T must be default-constructible (static_assert)
  template <typename AllocDummy = A, typename std::enable_if<std::is_default_constructible<AllocDummy>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR indirect() : ptr_(nullptr), alloc_()
  {
    static_assert(std::is_default_constructible<T>::value, "indirect: T must be default-constructible");
    allocate_and_construct();
  }

  // Mandate: T must be default-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, const A& a) : ptr_(nullptr), alloc_(a)
  {
    static_assert(std::is_default_constructible<T>::value, "indirect: T must be default-constructible");
    allocate_and_construct();
  }

  // Constraints: U not indirect, U not in_place_t, T constructible from U, A default-constructible (enable_if)
  template <class U = T, typename AllocDummy = A,
            typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, indirect>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, in_place_t>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_default_constructible<AllocDummy>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(U&& u) : ptr_(nullptr), alloc_()
  {
    allocate_and_construct(static_cast<U&&>(u));
  }

  // Constraints: U not indirect, U not in_place_t, T constructible from U (enable_if)
  template <class U = T,
            typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, indirect>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<!std::is_same<typename remove_cvref<U>::type, in_place_t>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_constructible<T, U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, const A& a, U&& u) : ptr_(nullptr), alloc_(a)
  {
    allocate_and_construct(static_cast<U&&>(u));
  }

  // Constraints: T constructible from Ts...; A must be default-constructible (enable_if)
  template <class... Ts, typename AllocDummy2 = A,
            typename std::enable_if<std::is_constructible<T, Ts...>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_default_constructible<AllocDummy2>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(in_place_t, Ts&&... ts) : ptr_(nullptr), alloc_()
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  // Constraint: T constructible from Ts... (enable_if)
  template <class... Ts, typename std::enable_if<std::is_constructible<T, Ts...>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, const A& a, in_place_t, Ts&&... ts) : ptr_(nullptr), alloc_(a)
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  // Mandate: T must be copy-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(const indirect& other)
      : ptr_(nullptr), alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_))
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  // Mandate: T must be copy-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(const indirect& other, std::allocator_arg_t, const A& a) : ptr_(nullptr), alloc_(a)
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  // (no constraint on T: move always works)
  YK_POLYFILL_CXX14_CONSTEXPR indirect(indirect&& other) noexcept : ptr_(other.ptr_), alloc_(static_cast<A&&>(other.alloc_))
  {
    other.ptr_ = nullptr;
  }

  // Mandate: T must be move-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect&& other, std::allocator_arg_t, const A& a)
      noexcept(indirect_detail::is_always_equal<A>::value)
      : ptr_(nullptr), alloc_(a)
  {
    static_assert(std::is_move_constructible<T>::value, "indirect: T must be move-constructible");
    indirect_detail::move_ctor_ops<indirect_detail::is_always_equal<A>::value>::apply(*this, static_cast<indirect&&>(other));
  }

  // --- Destructor ---

  YK_POLYFILL_CXX20_CONSTEXPR ~indirect() noexcept { destroy_owned(); }

  // --- Assignment ---

  // Mandates: T must be copy-constructible and copy-assignable (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(const indirect& other)
  {
    static_assert(std::is_copy_constructible<T>::value, "indirect: T must be copy-constructible");
    static_assert(std::is_copy_assignable<T>::value, "indirect: T must be copy-assignable");
    if (this == &other) return *this;
    indirect_detail::copy_assign_ops<alloc_traits::propagate_on_container_copy_assignment::value>::apply(*this, other);
    return *this;
  }

  // Mandate: T must be move-constructible (static_assert)
  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(indirect&& other)
      noexcept(alloc_traits::propagate_on_container_move_assignment::value
               || indirect_detail::is_always_equal<A>::value)
  {
    static_assert(std::is_move_constructible<T>::value, "indirect: T must be move-constructible");
    if (this == &other) return *this;
    indirect_detail::move_assign_ops<alloc_traits::propagate_on_container_move_assignment::value>::apply(*this, static_cast<indirect&&>(other));
    return *this;
  }

  // --- Observers ---

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T& operator*() & noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T& operator*() const& noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T&& operator*() && noexcept { return static_cast<T&&>(*ptr_); }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T&& operator*() const&& noexcept { return static_cast<const T&&>(*ptr_); }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T* operator->() noexcept { return ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T* operator->() const noexcept { return ptr_; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR bool valueless_after_move() const noexcept { return ptr_ == nullptr; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR A get_allocator() const noexcept { return alloc_; }

  // --- Swap ---

  YK_POLYFILL_CXX14_CONSTEXPR void swap(indirect& other)
      noexcept(alloc_traits::propagate_on_container_swap::value
               || indirect_detail::is_always_equal<A>::value)
  {
    indirect_detail::swap_ops<alloc_traits::propagate_on_container_swap::value>::apply(*this, other);
  }

  friend YK_POLYFILL_CXX14_CONSTEXPR void swap(indirect& a, indirect& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

  // --- Comparison ---

  template <class U, class AA>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator==(const indirect& lhs, const indirect<U, AA>& rhs)
      noexcept(noexcept(*lhs == *rhs))
  {
    if (lhs.valueless_after_move()) return rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return false;
    return *lhs == *rhs;
  }

  template <class U, class AA>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(const indirect& lhs, const indirect<U, AA>& rhs)
      noexcept(noexcept(*lhs != *rhs))
  {
    if (lhs.valueless_after_move()) return !rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return true;
    return *lhs != *rhs;
  }

  template <class U, typename std::enable_if<!indirect_detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator==(const indirect& lhs, const U& rhs)
      noexcept(noexcept(*lhs == rhs))
  {
    if (lhs.valueless_after_move()) return false;
    return *lhs == rhs;
  }

  template <class U, typename std::enable_if<!indirect_detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator==(const U& lhs, const indirect& rhs)
      noexcept(noexcept(lhs == *rhs))
  {
    if (rhs.valueless_after_move()) return false;
    return lhs == *rhs;
  }

  template <class U, typename std::enable_if<!indirect_detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(const indirect& lhs, const U& rhs)
      noexcept(noexcept(*lhs != rhs))
  {
    if (lhs.valueless_after_move()) return true;
    return *lhs != rhs;
  }

  template <class U, typename std::enable_if<!indirect_detail::is_indirect<U>::value, std::nullptr_t>::type = nullptr>
  friend YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(const U& lhs, const indirect& rhs)
      noexcept(noexcept(lhs != *rhs))
  {
    if (rhs.valueless_after_move()) return true;
    return lhs != *rhs;
  }

#if __cpp_lib_three_way_comparison >= 201907L
  template <class U, class AA>
  friend constexpr auto operator<=>(const indirect& lhs, const indirect<U, AA>& rhs)
      noexcept(noexcept(*lhs <=> *rhs))
  {
    if (lhs.valueless_after_move() && rhs.valueless_after_move()) return std::strong_ordering::equal;
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    if (rhs.valueless_after_move()) return std::strong_ordering::greater;
    return *lhs <=> *rhs;
  }

  template <class U>
  friend constexpr auto operator<=>(const indirect& lhs, const U& rhs)
      noexcept(noexcept(*lhs <=> rhs))
  {
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    return *lhs <=> rhs;
  }
#endif  // __cpp_lib_three_way_comparison
};

#if __cplusplus >= 201703L
// Deduction guides
template <class T>
indirect(T) -> indirect<T>;

template <class A, class T>
indirect(std::allocator_arg_t, A, T) -> indirect<T, typename std::allocator_traits<A>::template rebind_alloc<T>>;
#endif  // __cplusplus >= 201703L

// ---- ops specialisations (indirect is now complete) -------------------------

namespace indirect_detail {

template <>
struct swap_ops</*Pocs = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& a, indirect<T, A>& b) noexcept
  {
    cswap(a.alloc_, b.alloc_);
    cswap(a.ptr_, b.ptr_);
  }
};

template <>
struct swap_ops</*Pocs = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& a, indirect<T, A>& b) noexcept
  {
    cswap(a.ptr_, b.ptr_);
  }
};

template <>
struct copy_assign_ops</*Pocca = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, const indirect<T, A>& other)
  {
    if (self.alloc_ != other.alloc_) {
      self.destroy_owned();
      self.alloc_ = other.alloc_;
      if (other.ptr_ != nullptr) self.allocate_and_construct(*other.ptr_);
    } else {
      self.alloc_ = other.alloc_;
      self.copy_assign_content(other);
    }
  }
};

template <>
struct copy_assign_ops</*Pocca = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, const indirect<T, A>& other)
  {
    self.copy_assign_content(other);
  }
};

template <>
struct move_assign_ops</*Pocma = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.alloc_ = static_cast<A&&>(other.alloc_);
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template <>
struct move_assign_ops</*Pocma = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other)
      noexcept(is_always_equal<A>::value)
  {
    move_assign_ne_ops<is_always_equal<A>::value>::apply(self, static_cast<indirect<T, A>&&>(other));
  }
};

template <>
struct move_assign_ne_ops</*AlwaysEqual = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template <>
struct move_assign_ne_ops</*AlwaysEqual = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other)
  {
    if (self.alloc_ == other.alloc_) {
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

template <>
struct move_ctor_ops</*AlwaysEqual = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other) noexcept
  {
    self.ptr_ = other.ptr_;
    other.ptr_ = nullptr;
  }
};

template <>
struct move_ctor_ops</*AlwaysEqual = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(indirect<T, A>& self, indirect<T, A>&& other)
  {
    if (self.alloc_ == other.alloc_) {
      self.ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else if (other.ptr_ != nullptr) {
      self.allocate_and_construct(static_cast<T&&>(*other.ptr_));
    }
  }
};

}  // namespace indirect_detail

}  // namespace polyfill

}  // namespace yk

// Hash specialization
namespace std {

template <class T, class A>
struct hash<yk::polyfill::indirect<T, A>> {
  std::size_t operator()(const yk::polyfill::indirect<T, A>& ind) const
      noexcept(noexcept(std::hash<T>{}(*ind)))
  {
    if (ind.valueless_after_move()) return std::hash<T*>{}(nullptr);
    return std::hash<T>{}(*ind);
  }
};

}  // namespace std

#endif  // YK_POLYFILL_INDIRECT_HPP
