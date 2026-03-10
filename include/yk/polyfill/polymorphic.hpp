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
#if __cpp_lib_three_way_comparison >= 201907L
#include <compare>
#endif

namespace yk {

namespace polyfill {

template <class T, class A>
class polymorphic;

namespace polymorphic_detail {

template <bool Pocs>        struct swap_ops;
template <bool Pocca>       struct copy_assign_ops;
template <bool Pocma>       struct move_assign_ops;
template <bool AlwaysEqual> struct move_assign_ne_ops;
template <bool AlwaysEqual> struct move_ctor_ops;

}  // namespace polymorphic_detail

template <class T, class A = std::allocator<T>>
class polymorphic {
  static_assert(!std::is_array<T>::value, "polymorphic: T must not be an array type");

  struct holder_base {
    T* ptr_ = nullptr;
    virtual YK_POLYFILL_CXX20_CONSTEXPR holder_base* clone(A& alloc) const = 0;
    virtual YK_POLYFILL_CXX20_CONSTEXPR holder_base* move_clone(A& alloc) = 0;
    virtual YK_POLYFILL_CXX20_CONSTEXPR_VDESTROY void destroy(A& alloc) noexcept = 0;
    virtual YK_POLYFILL_CXX20_CONSTEXPR ~holder_base() noexcept = default;
  };

  template <class U>
  struct holder : holder_base {
    U obj_;

    template <class... Ts>
    YK_POLYFILL_CXX20_CONSTEXPR explicit holder(Ts&&... ts)
        noexcept(std::is_nothrow_constructible<U, Ts&&...>::value)
        : obj_(static_cast<Ts&&>(ts)...)
    {
      this->ptr_ = std::addressof(obj_);
    }

    YK_POLYFILL_CXX20_CONSTEXPR holder(const holder& other)
        noexcept(std::is_nothrow_copy_constructible<U>::value)
        : obj_(other.obj_)
    {
      this->ptr_ = std::addressof(obj_);
    }

    YK_POLYFILL_CXX20_CONSTEXPR ~holder() noexcept override = default;

    using holder_alloc_t = typename std::allocator_traits<A>::template rebind_alloc<holder<U>>;
    using holder_traits_t = std::allocator_traits<holder_alloc_t>;

    YK_POLYFILL_CXX20_CONSTEXPR holder_base* clone(A& alloc) const override
    {
      holder_alloc_t holder_alloc(alloc);
      holder<U>* p = holder_traits_t::allocate(holder_alloc, 1);
      try {
        holder_traits_t::construct(holder_alloc, p, *this);
      } catch (...) {
        holder_traits_t::deallocate(holder_alloc, p, 1);
        throw;
      }
      return p;
    }

    YK_POLYFILL_CXX20_CONSTEXPR holder_base* move_clone(A& alloc) override
    {
      holder_alloc_t holder_alloc(alloc);
      holder<U>* p = holder_traits_t::allocate(holder_alloc, 1);
      try {
        holder_traits_t::construct(holder_alloc, p, static_cast<U&&>(obj_));
      } catch (...) {
        holder_traits_t::deallocate(holder_alloc, p, 1);
        throw;
      }
      return p;
    }

    YK_POLYFILL_CXX20_CONSTEXPR_VDESTROY void destroy(A& alloc) noexcept override
    {
      holder_alloc_t holder_alloc(alloc);
      holder_traits_t::destroy(holder_alloc, this);
      holder_traits_t::deallocate(holder_alloc, this, 1);
    }
  };

  holder_base* holder_;
  YK_POLYFILL_NO_UNIQUE_ADDRESS A alloc_;

  template <class U, class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR void allocate_holder(Ts&&... ts)
  {
    using holder_alloc_t = typename std::allocator_traits<A>::template rebind_alloc<holder<U>>;
    using holder_traits_t = std::allocator_traits<holder_alloc_t>;
    holder_alloc_t holder_alloc(alloc_);
    holder<U>* p = holder_traits_t::allocate(holder_alloc, 1);
    try {
      holder_traits_t::construct(holder_alloc, p, static_cast<Ts&&>(ts)...);
    } catch (...) {
      holder_traits_t::deallocate(holder_alloc, p, 1);
      throw;
    }
    holder_ = p;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void destroy_owned() noexcept
  {
    if (holder_ == nullptr) return;
    holder_->destroy(alloc_);
    holder_ = nullptr;
  }

  YK_POLYFILL_CXX20_CONSTEXPR void copy_assign_content(const polymorphic& other)
  {
    destroy_owned();
    if (other.holder_ != nullptr) {
      holder_ = other.holder_->clone(alloc_);
    }
  }

  template <bool> friend struct polymorphic_detail::swap_ops;
  template <bool> friend struct polymorphic_detail::copy_assign_ops;
  template <bool> friend struct polymorphic_detail::move_assign_ops;
  template <bool> friend struct polymorphic_detail::move_assign_ne_ops;
  template <bool> friend struct polymorphic_detail::move_ctor_ops;

 public:
  using value_type = T;
  using allocator_type = A;

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic() : holder_(nullptr), alloc_()
  {
    static_assert(std::is_default_constructible<T>::value, "polymorphic: T must be default-constructible");
    static_assert(std::is_copy_constructible<T>::value, "polymorphic: T must be copy-constructible");
    allocate_holder<T>();
  }

  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a) : holder_(nullptr), alloc_(a)
  {
    static_assert(std::is_default_constructible<T>::value, "polymorphic: T must be default-constructible");
    static_assert(std::is_copy_constructible<T>::value, "polymorphic: T must be copy-constructible");
    allocate_holder<T>();
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(in_place_t, Ts&&... ts) : holder_(nullptr), alloc_()
  {
    allocate_holder<T>(static_cast<Ts&&>(ts)...);
  }

  template <class... Ts>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a, in_place_t, Ts&&... ts)
      : holder_(nullptr), alloc_(a)
  {
    allocate_holder<T>(static_cast<Ts&&>(ts)...);
  }

  template <class U, class... Ts,
            typename std::enable_if<std::is_base_of<T, U>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_constructible<U, Ts...>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_copy_constructible<U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(in_place_type_t<U>, Ts&&... ts) : holder_(nullptr), alloc_()
  {
    allocate_holder<U>(static_cast<Ts&&>(ts)...);
  }

  template <class U, class... Ts,
            typename std::enable_if<std::is_base_of<T, U>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_constructible<U, Ts...>::value, std::nullptr_t>::type = nullptr,
            typename std::enable_if<std::is_copy_constructible<U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR explicit polymorphic(std::allocator_arg_t, const A& a, in_place_type_t<U>, Ts&&... ts)
      : holder_(nullptr), alloc_(a)
  {
    allocate_holder<U>(static_cast<Ts&&>(ts)...);
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(const polymorphic& other)
      : holder_(nullptr), alloc_(std::allocator_traits<A>::select_on_container_copy_construction(other.alloc_))
  {
    if (other.holder_ != nullptr) {
      holder_ = other.holder_->clone(alloc_);
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(const polymorphic& other, std::allocator_arg_t, const A& a)
      : holder_(nullptr), alloc_(a)
  {
    if (other.holder_ != nullptr) {
      holder_ = other.holder_->clone(alloc_);
    }
  }

  YK_POLYFILL_CXX14_CONSTEXPR polymorphic(polymorphic&& other) noexcept
      : holder_(other.holder_), alloc_(static_cast<A&&>(other.alloc_))
  {
    other.holder_ = nullptr;
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic(polymorphic&& other, std::allocator_arg_t, const A& a)
      noexcept(indirect_detail::is_always_equal<A>::value)
      : holder_(nullptr), alloc_(a)
  {
    polymorphic_detail::move_ctor_ops<indirect_detail::is_always_equal<A>::value>::apply(*this, static_cast<polymorphic&&>(other));
  }

  YK_POLYFILL_CXX20_CONSTEXPR ~polymorphic() noexcept { destroy_owned(); }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic& operator=(const polymorphic& other)
  {
    if (this == &other) return *this;
    polymorphic_detail::copy_assign_ops<std::allocator_traits<A>::propagate_on_container_copy_assignment::value>::apply(*this, other);
    return *this;
  }

  YK_POLYFILL_CXX20_CONSTEXPR polymorphic& operator=(polymorphic&& other)
      noexcept(std::allocator_traits<A>::propagate_on_container_move_assignment::value
               || indirect_detail::is_always_equal<A>::value)
  {
    if (this == &other) return *this;
    polymorphic_detail::move_assign_ops<std::allocator_traits<A>::propagate_on_container_move_assignment::value>::apply(*this, static_cast<polymorphic&&>(other));
    return *this;
  }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T& operator*() & noexcept { return *holder_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T& operator*() const& noexcept { return *holder_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T&& operator*() && noexcept { return static_cast<T&&>(*holder_->ptr_); }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T&& operator*() const&& noexcept { return static_cast<const T&&>(*holder_->ptr_); }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR T* operator->() noexcept { return holder_->ptr_; }
  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR const T* operator->() const noexcept { return holder_->ptr_; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR bool valueless_after_move() const noexcept { return holder_ == nullptr; }

  [[nodiscard]] YK_POLYFILL_CXX14_CONSTEXPR A get_allocator() const noexcept { return alloc_; }

  YK_POLYFILL_CXX14_CONSTEXPR void swap(polymorphic& other)
      noexcept(std::allocator_traits<A>::propagate_on_container_swap::value
               || indirect_detail::is_always_equal<A>::value)
  {
    polymorphic_detail::swap_ops<std::allocator_traits<A>::propagate_on_container_swap::value>::apply(*this, other);
  }

  friend YK_POLYFILL_CXX14_CONSTEXPR void swap(polymorphic& a, polymorphic& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

// ---- Allocator-aware operations (defined after polymorphic is complete) -----

namespace polymorphic_detail {

template <>
struct swap_ops</*Pocs = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(polymorphic<T, A>& a, polymorphic<T, A>& b) noexcept
  {
    indirect_detail::cswap(a.alloc_, b.alloc_);
    indirect_detail::cswap(a.holder_, b.holder_);
  }
};

template <>
struct swap_ops</*Pocs = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(polymorphic<T, A>& a, polymorphic<T, A>& b) noexcept
  {
    indirect_detail::cswap(a.holder_, b.holder_);
  }
};

template <>
struct copy_assign_ops</*Pocca = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, const polymorphic<T, A>& other)
  {
    // polymorphic always destroys before cloning (no in-place reuse), so both
    // branches reduce to the same sequence: destroy, propagate alloc, clone.
    self.destroy_owned();
    self.alloc_ = other.alloc_;
    if (other.holder_ != nullptr) self.holder_ = other.holder_->clone(self.alloc_);
  }
};

template <>
struct copy_assign_ops</*Pocca = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, const polymorphic<T, A>& other)
  {
    self.copy_assign_content(other);
  }
};

template <>
struct move_assign_ops</*Pocma = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.alloc_ = static_cast<A&&>(other.alloc_);
    self.holder_ = other.holder_;
    other.holder_ = nullptr;
  }
};

template <>
struct move_assign_ops</*Pocma = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other)
      noexcept(indirect_detail::is_always_equal<A>::value)
  {
    move_assign_ne_ops<indirect_detail::is_always_equal<A>::value>::apply(self, static_cast<polymorphic<T, A>&&>(other));
  }
};

template <>
struct move_assign_ne_ops</*AlwaysEqual = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other) noexcept
  {
    self.destroy_owned();
    self.holder_ = other.holder_;
    other.holder_ = nullptr;
  }
};

template <>
struct move_assign_ne_ops</*AlwaysEqual = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other)
  {
    self.destroy_owned();
    if (self.alloc_ == other.alloc_) {
      self.holder_ = other.holder_;
      other.holder_ = nullptr;
    } else if (other.holder_ != nullptr) {
      self.holder_ = other.holder_->move_clone(self.alloc_);
    }
  }
};

template <>
struct move_ctor_ops</*AlwaysEqual = */ true> {
  template <class T, class A>
  static YK_POLYFILL_CXX14_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other) noexcept
  {
    self.holder_ = other.holder_;
    other.holder_ = nullptr;
  }
};

template <>
struct move_ctor_ops</*AlwaysEqual = */ false> {
  template <class T, class A>
  static YK_POLYFILL_CXX20_CONSTEXPR void apply(polymorphic<T, A>& self, polymorphic<T, A>&& other)
  {
    if (self.alloc_ == other.alloc_) {
      self.holder_ = other.holder_;
      other.holder_ = nullptr;
    } else if (other.holder_ != nullptr) {
      self.holder_ = other.holder_->move_clone(self.alloc_);
    }
  }
};

}  // namespace polymorphic_detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_POLYFILL_POLYMORPHIC_HPP
