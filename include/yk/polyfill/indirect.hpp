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

// Forward declaration so is_indirect can be defined before the class body
template <class T, class A>
class indirect;

template <class T>
struct is_indirect : std::false_type {};

template <class T, class A>
struct is_indirect<indirect<T, A>> : std::true_type {};

namespace indirect_detail {

// Fallback for is_always_equal (added to allocator_traits in C++17)
template <class A, class = void>
struct is_always_equal : std::is_empty<A> {};

#if __cpp_lib_allocator_traits_is_always_equal >= 201411L
template <class A>
struct is_always_equal<A, typename std::enable_if<std::allocator_traits<A>::is_always_equal::value>::type> : std::true_type {};
#endif

}  // namespace indirect_detail

template <class T, class A = std::allocator<T>>
class indirect {
  static_assert(!std::is_array<T>::value, "indirect: T must not be an array type");

  using alloc_traits = std::allocator_traits<A>;

  T* ptr_;
  YK_POLYFILL_NO_UNIQUE_ADDRESS A alloc_;

  YK_POLYFILL_CXX20_CONSTEXPR void allocate_and_construct_default()
  {
    ptr_ = alloc_traits::allocate(alloc_, 1);
    try {
      alloc_traits::construct(alloc_, ptr_);
    } catch (...) {
      alloc_traits::deallocate(alloc_, ptr_, 1);
      ptr_ = nullptr;
      throw;
    }
  }

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

 public:
  using value_type = T;
  using allocator_type = A;

  // --- Constructors ---

  YK_POLYFILL_CXX20_CONSTEXPR indirect() : ptr_(nullptr), alloc_()
  {
    allocate_and_construct_default();
  }

  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, const A& a) : ptr_(nullptr), alloc_(a)
  {
    allocate_and_construct_default();
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(in_place_t, Ts&&... ts) : ptr_(nullptr), alloc_()
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit indirect(std::allocator_arg_t, const A& a, in_place_t, Ts&&... ts) : ptr_(nullptr), alloc_(a)
  {
    allocate_and_construct(static_cast<Ts&&>(ts)...);
  }

  // Copy constructor
  YK_POLYFILL_CXX20_CONSTEXPR indirect(const indirect& other)
      : ptr_(nullptr), alloc_(alloc_traits::select_on_container_copy_construction(other.alloc_))
  {
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR indirect(const indirect& other, std::allocator_arg_t, const A& a) : ptr_(nullptr), alloc_(a)
  {
    if (other.ptr_ != nullptr) {
      allocate_and_construct(*other.ptr_);
    }
  }

  // Move constructor
  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect&& other) noexcept : ptr_(other.ptr_), alloc_(static_cast<A&&>(other.alloc_))
  {
    other.ptr_ = nullptr;
  }

  YK_POLYFILL_CXX20_CONSTEXPR indirect(indirect&& other, std::allocator_arg_t, const A& a)
      noexcept(indirect_detail::is_always_equal<A>::value)
      : ptr_(nullptr), alloc_(a)
  {
    if (indirect_detail::is_always_equal<A>::value || alloc_ == other.alloc_) {
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else {
      if (other.ptr_ != nullptr) {
        allocate_and_construct(static_cast<T&&>(*other.ptr_));
      }
    }
  }

  // --- Destructor ---

  YK_POLYFILL_CXX20_CONSTEXPR ~indirect() noexcept { destroy_owned(); }

  // --- Assignment ---

  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(const indirect& other)
  {
    if (this == &other) return *this;

    if (alloc_traits::propagate_on_container_copy_assignment::value && alloc_ != other.alloc_) {
      destroy_owned();
      alloc_ = other.alloc_;
      if (other.ptr_ != nullptr) {
        allocate_and_construct(*other.ptr_);
      }
      return *this;
    }

    if (alloc_traits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }

    if (other.ptr_ == nullptr) {
      destroy_owned();
    } else if (ptr_ != nullptr) {
      *ptr_ = *other.ptr_;
    } else {
      allocate_and_construct(*other.ptr_);
    }
    return *this;
  }

  YK_POLYFILL_CXX20_CONSTEXPR indirect& operator=(indirect&& other)
      noexcept(alloc_traits::propagate_on_container_move_assignment::value
               || indirect_detail::is_always_equal<A>::value)
  {
    if (this == &other) return *this;

    if (alloc_traits::propagate_on_container_move_assignment::value) {
      destroy_owned();
      alloc_ = static_cast<A&&>(other.alloc_);
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else if (indirect_detail::is_always_equal<A>::value || alloc_ == other.alloc_) {
      destroy_owned();
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    } else {
      if (other.ptr_ != nullptr) {
        if (ptr_ != nullptr) {
          *ptr_ = static_cast<T&&>(*other.ptr_);
        } else {
          allocate_and_construct(static_cast<T&&>(*other.ptr_));
        }
      } else {
        destroy_owned();
      }
    }
    return *this;
  }

  // --- Observers ---

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T& operator*() & noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T& operator*() const& noexcept { return *ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T&& operator*() && noexcept { return static_cast<T&&>(*ptr_); }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T&& operator*() const&& noexcept { return static_cast<const T&&>(*ptr_); }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T* operator->() noexcept { return ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T* operator->() const noexcept { return ptr_; }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR bool valueless_after_move() const noexcept { return ptr_ == nullptr; }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR A get_allocator() const noexcept { return alloc_; }

  // --- Swap ---

  YK_POLYFILL_CXX20_CONSTEXPR void swap(indirect& other)
      noexcept(alloc_traits::propagate_on_container_swap::value
               || indirect_detail::is_always_equal<A>::value)
  {
    using std::swap;
    if (alloc_traits::propagate_on_container_swap::value) {
      swap(alloc_, other.alloc_);
    }
    swap(ptr_, other.ptr_);
  }

  friend YK_POLYFILL_CXX20_CONSTEXPR void swap(indirect& a, indirect& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

  // --- Comparison ---

  template <class U, class AA>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const indirect& lhs, const indirect<U, AA>& rhs)
  {
    if (lhs.valueless_after_move()) return rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return false;
    return *lhs == *rhs;
  }

  template <class U, class AA>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const indirect& lhs, const indirect<U, AA>& rhs)
  {
    return !(lhs == rhs);
  }

  template <class U, class = typename std::enable_if<!is_indirect<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const indirect& lhs, const U& rhs)
  {
    if (lhs.valueless_after_move()) return false;
    return *lhs == rhs;
  }

  template <class U, class = typename std::enable_if<!is_indirect<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const U& lhs, const indirect& rhs)
  {
    if (rhs.valueless_after_move()) return false;
    return lhs == *rhs;
  }

  template <class U, class = typename std::enable_if<!is_indirect<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const indirect& lhs, const U& rhs)
  {
    return !(lhs == rhs);
  }

  template <class U, class = typename std::enable_if<!is_indirect<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const U& lhs, const indirect& rhs)
  {
    return !(lhs == rhs);
  }

#if __cpp_lib_three_way_comparison >= 201907L
  template <class U, class AA>
  friend constexpr auto operator<=>(const indirect& lhs, const indirect<U, AA>& rhs)
  {
    if (lhs.valueless_after_move() && rhs.valueless_after_move()) return std::strong_ordering::equal;
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    if (rhs.valueless_after_move()) return std::strong_ordering::greater;
    return *lhs <=> *rhs;
  }

  template <class U>
  friend constexpr auto operator<=>(const indirect& lhs, const U& rhs)
  {
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    return *lhs <=> rhs;
  }
#endif  // __cpp_lib_three_way_comparison
};

// Deduction guide
template <class T>
indirect(T) -> indirect<T>;

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
