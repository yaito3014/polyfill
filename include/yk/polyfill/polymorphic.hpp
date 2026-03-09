#ifndef YK_POLYFILL_POLYMORPHIC_HPP
#define YK_POLYFILL_POLYMORPHIC_HPP

// polymorphic<T, A>: value-semantic wrapper for a heap-allocated T that may
// be a derived type.  Copies preserve the dynamic type via type-erased clone.
// Polyfill of P3019R14 std::polymorphic for C++11 and later.

#include <yk/polyfill/config.hpp>
#include <yk/polyfill/indirect.hpp>  // for indirect_detail::is_always_equal
#include <yk/polyfill/utility.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

// Forward declaration for the constraint helper
template <class T, class A>
class polymorphic;

template <class T>
struct is_polymorphic_wrapper : std::false_type {};

template <class T, class A>
struct is_polymorphic_wrapper<polymorphic<T, A>> : std::true_type {};

template <class T, class A = std::allocator<T>>
class polymorphic {
  static_assert(!std::is_array<T>::value, "polymorphic: T must not be an array type");

  // ---- Control block -------------------------------------------------------
  // cb_base is type-erased: it knows how to clone and destroy the stored object
  // without knowing U (the actual dynamic type).

  struct cb_base {
    T* ptr_;  // pointer to the owned T (actually U) object
    virtual YK_POLYFILL_CXX20_CONSTEXPR cb_base* clone(A& alloc) const = 0;
    virtual YK_POLYFILL_CXX20_CONSTEXPR void destroy(A& alloc) noexcept = 0;
    // Public virtual destructor: used by delete cb_ in the constexpr path.
    virtual YK_POLYFILL_CXX20_CONSTEXPR ~cb_base() = default;
  };

  template <class U>
  struct cb : cb_base {
    // Store U as a direct typed member so std::construct_at can be used in
    // constexpr contexts (placement new on void* is not constexpr).
    U obj_;

    template <class... Ts>
    YK_POLYFILL_CXX20_CONSTEXPR explicit cb(Ts&&... ts) : obj_(static_cast<Ts&&>(ts)...)
    {
      this->ptr_ = &obj_;
    }

    // Copy-construct from another cb<U> (used in clone())
    YK_POLYFILL_CXX20_CONSTEXPR cb(const cb& other) : obj_(other.obj_)
    {
      this->ptr_ = &obj_;
    }

    YK_POLYFILL_CXX20_CONSTEXPR ~cb() override = default;

    using cb_alloc_t = typename std::allocator_traits<A>::template rebind_alloc<cb<U>>;
    using cb_traits_t = std::allocator_traits<cb_alloc_t>;

    YK_POLYFILL_CXX20_CONSTEXPR cb_base* clone(A& alloc) const override
    {
#if __cpp_lib_is_constant_evaluated >= 201811L
      if (std::is_constant_evaluated()) {
        return new cb<U>(*this);
      }
#endif
      cb_alloc_t cb_alloc(alloc);
      cb<U>* new_cb = cb_traits_t::allocate(cb_alloc, 1);
      try {
        cb_traits_t::construct(cb_alloc, new_cb, *this);
      } catch (...) {
        cb_traits_t::deallocate(cb_alloc, new_cb, 1);
        throw;
      }
      return new_cb;
    }

    YK_POLYFILL_CXX20_CONSTEXPR void destroy(A& alloc) noexcept override
    {
      cb_alloc_t cb_alloc(alloc);
      cb_traits_t::destroy(cb_alloc, this);
      cb_traits_t::deallocate(cb_alloc, this, 1);
    }
  };

  // ---- Data members --------------------------------------------------------

  cb_base* cb_;
  YK_POLYFILL_NO_UNIQUE_ADDRESS A alloc_;

  // ---- Private helpers -----------------------------------------------------

  template <class U, class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR void allocate_cb(Ts&&... ts)
  {
#if __cpp_lib_is_constant_evaluated >= 201811L
    if (std::is_constant_evaluated()) {
      cb_ = new cb<U>(static_cast<Ts&&>(ts)...);
      return;
    }
#endif
    using cb_alloc_t = typename std::allocator_traits<A>::template rebind_alloc<cb<U>>;
    using cb_traits_t = std::allocator_traits<cb_alloc_t>;
    cb_alloc_t cb_alloc(alloc_);
    cb<U>* new_cb = cb_traits_t::allocate(cb_alloc, 1);
    try {
      cb_traits_t::construct(cb_alloc, new_cb, static_cast<Ts&&>(ts)...);
    } catch (...) {
      cb_traits_t::deallocate(cb_alloc, new_cb, 1);
      throw;
    }
    cb_ = new_cb;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy_owned() noexcept
  {
    if (cb_ == nullptr) return;
#if __cpp_lib_is_constant_evaluated >= 201811L
    if (std::is_constant_evaluated()) {
      delete cb_;
      cb_ = nullptr;
      return;
    }
#endif
    cb_->destroy(alloc_);
    cb_ = nullptr;
  }

 public:
  using value_type = T;
  using allocator_type = A;

  // --- Constructors ---

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic() : cb_(nullptr), alloc_()
  {
    allocate_cb<T>();
  }

  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a) : cb_(nullptr), alloc_(a)
  {
    allocate_cb<T>();
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(in_place_t, Ts&&... ts) : cb_(nullptr), alloc_()
  {
    allocate_cb<T>(static_cast<Ts&&>(ts)...);
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a, in_place_t, Ts&&... ts)
      : cb_(nullptr), alloc_(a)
  {
    allocate_cb<T>(static_cast<Ts&&>(ts)...);
  }

  // Construct from a derived type U
  template <class U, class... Ts,
            class = typename std::enable_if<std::is_base_of<T, U>::value && !std::is_same<T, U>::value>::type>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(in_place_type_t<U>, Ts&&... ts) : cb_(nullptr), alloc_()
  {
    allocate_cb<U>(static_cast<Ts&&>(ts)...);
  }

  template <class U, class... Ts,
            class = typename std::enable_if<std::is_base_of<T, U>::value && !std::is_same<T, U>::value>::type>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a, in_place_type_t<U>, Ts&&... ts)
      : cb_(nullptr), alloc_(a)
  {
    allocate_cb<U>(static_cast<Ts&&>(ts)...);
  }

  // Copy constructor
  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(const polymorphic& other)
      : cb_(nullptr), alloc_(std::allocator_traits<A>::select_on_container_copy_construction(other.alloc_))
  {
    if (other.cb_ != nullptr) {
      cb_ = other.cb_->clone(alloc_);
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(const polymorphic& other, std::allocator_arg_t, const A& a)
      : cb_(nullptr), alloc_(a)
  {
    if (other.cb_ != nullptr) {
      cb_ = other.cb_->clone(alloc_);
    }
  }

  // Move constructor
  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(polymorphic&& other) noexcept
      : cb_(other.cb_), alloc_(static_cast<A&&>(other.alloc_))
  {
    other.cb_ = nullptr;
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(polymorphic&& other, std::allocator_arg_t, const A& a)
      noexcept(indirect_detail::is_always_equal<A>::value)
      : cb_(nullptr), alloc_(a)
  {
    if (indirect_detail::is_always_equal<A>::value || alloc_ == other.alloc_) {
      cb_ = other.cb_;
      other.cb_ = nullptr;
    } else {
      if (other.cb_ != nullptr) {
        // Can't cheaply move with different allocators; clone instead
        cb_ = other.cb_->clone(alloc_);
      }
    }
  }

  // --- Destructor ---

  YK_POLYFILL_CXX20_CONSTEXPR ~polymorphic() noexcept { destroy_owned(); }

  // --- Assignment ---

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic& operator=(const polymorphic& other)
  {
    if (this == &other) return *this;

    if (std::allocator_traits<A>::propagate_on_container_copy_assignment::value && alloc_ != other.alloc_) {
      destroy_owned();
      alloc_ = other.alloc_;
      if (other.cb_ != nullptr) {
        cb_ = other.cb_->clone(alloc_);
      }
      return *this;
    }

    if (std::allocator_traits<A>::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }

    destroy_owned();
    if (other.cb_ != nullptr) {
      cb_ = other.cb_->clone(alloc_);
    }
    return *this;
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic& operator=(polymorphic&& other)
      noexcept(std::allocator_traits<A>::propagate_on_container_move_assignment::value
               || indirect_detail::is_always_equal<A>::value)
  {
    if (this == &other) return *this;

    if (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
      destroy_owned();
      alloc_ = static_cast<A&&>(other.alloc_);
      cb_ = other.cb_;
      other.cb_ = nullptr;
    } else if (indirect_detail::is_always_equal<A>::value || alloc_ == other.alloc_) {
      destroy_owned();
      cb_ = other.cb_;
      other.cb_ = nullptr;
    } else {
      destroy_owned();
      if (other.cb_ != nullptr) {
        cb_ = other.cb_->clone(alloc_);
      }
    }
    return *this;
  }

  // --- Observers ---

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T& operator*() & noexcept { return *cb_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T& operator*() const& noexcept { return *cb_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T&& operator*() && noexcept { return static_cast<T&&>(*cb_->ptr_); }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T&& operator*() const&& noexcept { return static_cast<const T&&>(*cb_->ptr_); }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR T* operator->() noexcept { return cb_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR const T* operator->() const noexcept { return cb_->ptr_; }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR bool valueless_after_move() const noexcept { return cb_ == nullptr; }

  [[nodiscard]] YK_POLYFILL_CXX20_CONSTEXPR A get_allocator() const noexcept { return alloc_; }

  // --- Swap ---

  YK_POLYFILL_CXX20_CONSTEXPR void swap(polymorphic& other)
      noexcept(std::allocator_traits<A>::propagate_on_container_swap::value
               || indirect_detail::is_always_equal<A>::value)
  {
    using std::swap;
    if (std::allocator_traits<A>::propagate_on_container_swap::value) {
      swap(alloc_, other.alloc_);
    }
    swap(cb_, other.cb_);
  }

  friend YK_POLYFILL_CXX20_CONSTEXPR void swap(polymorphic& a, polymorphic& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }

  // --- Comparison ---

  template <class U, class AA>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const polymorphic& lhs, const polymorphic<U, AA>& rhs)
  {
    if (lhs.valueless_after_move()) return rhs.valueless_after_move();
    if (rhs.valueless_after_move()) return false;
    return *lhs == *rhs;
  }

  template <class U, class AA>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const polymorphic& lhs, const polymorphic<U, AA>& rhs)
  {
    return !(lhs == rhs);
  }

  template <class U, class = typename std::enable_if<!is_polymorphic_wrapper<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const polymorphic& lhs, const U& rhs)
  {
    if (lhs.valueless_after_move()) return false;
    return *lhs == rhs;
  }

  template <class U, class = typename std::enable_if<!is_polymorphic_wrapper<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator==(const U& lhs, const polymorphic& rhs)
  {
    if (rhs.valueless_after_move()) return false;
    return lhs == *rhs;
  }

  template <class U, class = typename std::enable_if<!is_polymorphic_wrapper<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const polymorphic& lhs, const U& rhs)
  {
    return !(lhs == rhs);
  }

  template <class U, class = typename std::enable_if<!is_polymorphic_wrapper<U>::value>::type>
  friend YK_POLYFILL_CXX20_CONSTEXPR bool operator!=(const U& lhs, const polymorphic& rhs)
  {
    return !(lhs == rhs);
  }

#if __cpp_lib_three_way_comparison >= 201907L
  template <class U, class AA>
  friend constexpr auto operator<=>(const polymorphic& lhs, const polymorphic<U, AA>& rhs)
  {
    if (lhs.valueless_after_move() && rhs.valueless_after_move()) return std::strong_ordering::equal;
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    if (rhs.valueless_after_move()) return std::strong_ordering::greater;
    return *lhs <=> *rhs;
  }

  template <class U>
  friend constexpr auto operator<=>(const polymorphic& lhs, const U& rhs)
  {
    if (lhs.valueless_after_move()) return std::strong_ordering::less;
    return *lhs <=> rhs;
  }
#endif  // __cpp_lib_three_way_comparison
};

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_POLYMORPHIC_HPP
