#ifndef YK_ZZ_POLYFILL_MEMORY_HPP
#define YK_ZZ_POLYFILL_MEMORY_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/bits/swap.hpp>
#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/extension/ebo_storage.hpp>
#include <yk/polyfill/type_traits.hpp>

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <cstddef>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace yk {

namespace polyfill {

// construct_at

template<class T, class... Args>
YK_POLYFILL_CXX20_CONSTEXPR T* construct_at(T* dest, Args&&... args)
{
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
  return std::construct_at(dest, std::forward<Args>(args)...);
#else
  return ::new (static_cast<void*>(dest)) T(std::forward<Args>(args)...);
#endif
}

// detail

namespace detail {

// Detect Deleter::pointer, fallback to T*
template<class T, class Deleter, class = void>
struct unique_ptr_pointer {
  using type = T*;
};

template<class T, class Deleter>
struct unique_ptr_pointer<T, Deleter, void_t<typename std::remove_reference<Deleter>::type::pointer>> {
  using type = typename std::remove_reference<Deleter>::type::pointer;
};

}  // namespace detail

// default_delete - primary template

template<class T>
struct default_delete {
  constexpr default_delete() noexcept = default;

  template<class U, typename std::enable_if<std::is_convertible<U*, T*>::value, std::nullptr_t>::type = nullptr>
  constexpr default_delete(default_delete<U> const&) noexcept
  {
  }

  YK_POLYFILL_CXX20_CONSTEXPR void operator()(T* ptr) const noexcept { delete ptr; }
};

// default_delete - array specialization

template<class T>
struct default_delete<T[]> {
  constexpr default_delete() noexcept = default;

  template<class U, typename std::enable_if<std::is_convertible<U (*)[], T (*)[]>::value, std::nullptr_t>::type = nullptr>
  constexpr default_delete(default_delete<U[]> const&) noexcept
  {
  }

  template<class U, typename std::enable_if<std::is_convertible<U (*)[], T (*)[]>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR void operator()(U* ptr) const noexcept
  {
    delete[] ptr;
  }
};

// unique_ptr - primary template

template<class T, class Deleter = default_delete<T>>
class unique_ptr : extension::ebo_storage<Deleter> {
  static_assert(!std::is_rvalue_reference<Deleter>::value, "Deleter must not be an rvalue reference type");

  using deleter_base = extension::ebo_storage<Deleter>;

public:
  using pointer = typename detail::unique_ptr_pointer<T, Deleter>::type;
  using element_type = T;
  using deleter_type = Deleter;

  // (1) Default constructor + nullptr constructor
  template<
      class D = Deleter,
      typename std::enable_if<conjunction<std::is_default_constructible<D>, negation<std::is_pointer<D>>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr() noexcept : deleter_base(), ptr_(pointer())
  {
  }

  template<
      class D = Deleter,
      typename std::enable_if<conjunction<std::is_default_constructible<D>, negation<std::is_pointer<D>>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr(std::nullptr_t) noexcept : deleter_base(), ptr_(pointer())
  {
  }

  // (2) Pointer constructor
  template<
      class D = Deleter,
      typename std::enable_if<conjunction<std::is_default_constructible<D>, negation<std::is_pointer<D>>>::value, std::nullptr_t>::type = nullptr>
  constexpr explicit unique_ptr(pointer p) noexcept : deleter_base(), ptr_(p)
  {
  }

  // (3a) Pointer + deleter (const lvalue ref; also handles reference deleters via collapsing)
  template<class D = Deleter, typename std::enable_if<std::is_constructible<D, D const&>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr(pointer p, Deleter const& d) noexcept : deleter_base(d), ptr_(p)
  {
  }

  // (3b) Pointer + deleter (non-reference deleter, rvalue ref)
  template<
      class D = Deleter,
      typename std::enable_if<conjunction<negation<std::is_reference<D>>, std::is_constructible<D, D&&>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr(pointer p, Deleter&& d) noexcept : deleter_base(std::forward<Deleter>(d)), ptr_(p)
  {
  }

  // Prevent rvalue binding for reference deleters
  template<class D = Deleter, typename std::enable_if<std::is_reference<D>::value, std::nullptr_t>::type = nullptr>
  unique_ptr(pointer, typename std::remove_reference<Deleter>::type&&) = delete;

  // (4) Move constructor
  template<class D = Deleter, typename std::enable_if<std::is_move_constructible<D>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR unique_ptr(unique_ptr&& u) noexcept : deleter_base(std::move(u.get_deleter())), ptr_(u.release())
  {
  }

  // (5) Converting move constructor
  template<
      class U, class E,
      typename std::enable_if<
          conjunction<
              std::is_convertible<typename unique_ptr<U, E>::pointer, pointer>, negation<std::is_array<U>>,
              typename std::conditional<std::is_reference<Deleter>::value, std::is_same<E, Deleter>, std::is_convertible<E, Deleter>>::type>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR unique_ptr(unique_ptr<U, E>&& u) noexcept : deleter_base(std::forward<E>(u.get_deleter())), ptr_(u.release())
  {
  }

  // Copy is deleted
  unique_ptr(unique_ptr const&) = delete;
  unique_ptr& operator=(unique_ptr const&) = delete;

  // Destructor
  YK_POLYFILL_CXX20_CONSTEXPR ~unique_ptr() noexcept
  {
    if (get()) {
      get_deleter()(get());
    }
  }

  // Move assignment
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(unique_ptr&& r) noexcept
  {
    reset(r.release());
    get_deleter() = std::move(r.get_deleter());
    return *this;
  }

  // Converting move assignment
  template<
      class U, class E,
      typename std::enable_if<
          conjunction<negation<std::is_array<U>>, std::is_convertible<typename unique_ptr<U, E>::pointer, pointer>, std::is_assignable<Deleter&, E&&>>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(unique_ptr<U, E>&& r) noexcept
  {
    reset(r.release());
    get_deleter() = std::forward<E>(r.get_deleter());
    return *this;
  }

  // nullptr assignment
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(std::nullptr_t) noexcept
  {
    reset();
    return *this;
  }

  // Observers

  YK_POLYFILL_CXX14_CONSTEXPR typename std::add_lvalue_reference<T>::type operator*() const noexcept(noexcept(*std::declval<pointer>())) { return *get(); }

  constexpr pointer operator->() const noexcept { return get(); }

  constexpr pointer get() const noexcept { return ptr_; }

  constexpr deleter_type const& get_deleter() const noexcept { return deleter_base::stored_value(); }
  YK_POLYFILL_CXX14_CONSTEXPR deleter_type& get_deleter() noexcept { return deleter_base::stored_value(); }

  constexpr explicit operator bool() const noexcept { return get() != pointer(); }

  // Modifiers

  YK_POLYFILL_CXX14_CONSTEXPR pointer release() noexcept { return polyfill::exchange(ptr_, pointer()); }

  YK_POLYFILL_CXX20_CONSTEXPR void reset(pointer p = pointer()) noexcept
  {
    if (pointer old = polyfill::exchange(ptr_, p)) {
      get_deleter()(old);
    }
  }

  YK_POLYFILL_CXX14_CONSTEXPR void swap(unique_ptr& other) noexcept(is_nothrow_swappable<Deleter>::value)
  {
    detail::constexpr_swap(ptr_, other.ptr_);
    detail::constexpr_swap(deleter_base::stored_value(), other.deleter_base::stored_value());
  }

private:
  pointer ptr_;
};

// unique_ptr - array specialization

template<class T, class Deleter>
class unique_ptr<T[], Deleter> : extension::ebo_storage<Deleter> {
  static_assert(!std::is_rvalue_reference<Deleter>::value, "Deleter must not be an rvalue reference type");

  using deleter_base = extension::ebo_storage<Deleter>;

public:
  using pointer = typename detail::unique_ptr_pointer<T, Deleter>::type;
  using element_type = T;
  using deleter_type = Deleter;

private:
  template<class U>
  using is_safe_array_convertible = conjunction<
      std::is_same<pointer, element_type*>, std::is_pointer<U>, std::is_convertible<typename std::remove_pointer<U>::type (*)[], element_type (*)[]>>;

  template<class U>
  using is_valid_pointer = disjunction<std::is_same<U, pointer>, std::is_same<U, std::nullptr_t>, is_safe_array_convertible<U>>;

public:
  // (1) Default constructor + nullptr constructor
  template<
      class D = Deleter,
      typename std::enable_if<conjunction<std::is_default_constructible<D>, negation<std::is_pointer<D>>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr() noexcept : deleter_base(), ptr_(pointer())
  {
  }

  template<
      class D = Deleter,
      typename std::enable_if<conjunction<std::is_default_constructible<D>, negation<std::is_pointer<D>>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr(std::nullptr_t) noexcept : deleter_base(), ptr_(pointer())
  {
  }

  // (2) Pointer constructor
  template<
      class U, class D = Deleter,
      typename std::enable_if<
          conjunction<
              std::is_default_constructible<D>, negation<std::is_pointer<D>>, disjunction<std::is_same<U, pointer>, is_safe_array_convertible<U>>>::value,
          std::nullptr_t>::type = nullptr>
  constexpr explicit unique_ptr(U p) noexcept : deleter_base(), ptr_(p)
  {
  }

  // (3a) Pointer + deleter (const lvalue ref; also handles reference deleters via collapsing)
  template<
      class U, class D = Deleter,
      typename std::enable_if<conjunction<std::is_constructible<D, D const&>, is_valid_pointer<U>>::value, std::nullptr_t>::type = nullptr>
  constexpr unique_ptr(U p, Deleter const& d) noexcept : deleter_base(d), ptr_(p)
  {
  }

  // (3b) Pointer + deleter (non-reference deleter, rvalue ref)
  template<
      class U, class D = Deleter,
      typename std::enable_if<conjunction<negation<std::is_reference<D>>, std::is_constructible<D, D&&>, is_valid_pointer<U>>::value, std::nullptr_t>::type =
          nullptr>
  constexpr unique_ptr(U p, Deleter&& d) noexcept : deleter_base(std::forward<Deleter>(d)), ptr_(p)
  {
  }

  // Prevent rvalue binding for reference deleters
  template<class U, class D = Deleter, typename std::enable_if<conjunction<std::is_reference<D>, is_valid_pointer<U>>::value, std::nullptr_t>::type = nullptr>
  unique_ptr(U, typename std::remove_reference<Deleter>::type&&) = delete;

  // (4) Move constructor
  template<class D = Deleter, typename std::enable_if<std::is_move_constructible<D>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR unique_ptr(unique_ptr&& u) noexcept : deleter_base(std::move(u.get_deleter())), ptr_(u.release())
  {
  }

  // (5) Converting move constructor
  template<
      class U, class E,
      typename std::enable_if<
          conjunction<
              std::is_array<U>, std::is_same<pointer, element_type*>,
              std::is_same<typename unique_ptr<U, E>::pointer, typename unique_ptr<U, E>::element_type*>,
              std::is_convertible<typename unique_ptr<U, E>::element_type (*)[], element_type (*)[]>,
              typename std::conditional<std::is_reference<Deleter>::value, std::is_same<E, Deleter>, std::is_convertible<E, Deleter>>::type>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX14_CONSTEXPR unique_ptr(unique_ptr<U, E>&& u) noexcept : deleter_base(std::forward<E>(u.get_deleter())), ptr_(u.release())
  {
  }

  // Copy is deleted
  unique_ptr(unique_ptr const&) = delete;
  unique_ptr& operator=(unique_ptr const&) = delete;

  // Destructor
  YK_POLYFILL_CXX20_CONSTEXPR ~unique_ptr() noexcept
  {
    if (get()) {
      get_deleter()(get());
    }
  }

  // Move assignment
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(unique_ptr&& r) noexcept
  {
    reset(r.release());
    get_deleter() = std::move(r.get_deleter());
    return *this;
  }

  // Converting move assignment
  template<
      class U, class E,
      typename std::enable_if<
          conjunction<
              std::is_array<U>, std::is_same<pointer, element_type*>,
              std::is_same<typename unique_ptr<U, E>::pointer, typename unique_ptr<U, E>::element_type*>,
              std::is_convertible<typename unique_ptr<U, E>::element_type (*)[], element_type (*)[]>, std::is_assignable<Deleter&, E&&>>::value,
          std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(unique_ptr<U, E>&& r) noexcept
  {
    reset(r.release());
    get_deleter() = std::forward<E>(r.get_deleter());
    return *this;
  }

  // nullptr assignment
  YK_POLYFILL_CXX20_CONSTEXPR unique_ptr& operator=(std::nullptr_t) noexcept
  {
    reset();
    return *this;
  }

  // Observers

  YK_POLYFILL_CXX14_CONSTEXPR typename std::add_lvalue_reference<T>::type operator[](std::size_t i) const noexcept { return get()[i]; }

  constexpr pointer get() const noexcept { return ptr_; }

  constexpr deleter_type const& get_deleter() const noexcept { return deleter_base::stored_value(); }
  YK_POLYFILL_CXX14_CONSTEXPR deleter_type& get_deleter() noexcept { return deleter_base::stored_value(); }

  constexpr explicit operator bool() const noexcept { return get() != pointer(); }

  // Modifiers

  YK_POLYFILL_CXX14_CONSTEXPR pointer release() noexcept { return polyfill::exchange(ptr_, pointer()); }

  template<class U, typename std::enable_if<is_valid_pointer<U>::value, std::nullptr_t>::type = nullptr>
  YK_POLYFILL_CXX20_CONSTEXPR void reset(U p) noexcept
  {
    if (pointer old = polyfill::exchange(ptr_, p)) {
      get_deleter()(old);
    }
  }

  YK_POLYFILL_CXX20_CONSTEXPR void reset(std::nullptr_t = nullptr) noexcept
  {
    if (pointer old = polyfill::exchange(ptr_, pointer())) {
      get_deleter()(old);
    }
  }

  YK_POLYFILL_CXX14_CONSTEXPR void swap(unique_ptr& other) noexcept(is_nothrow_swappable<Deleter>::value)
  {
    detail::constexpr_swap(ptr_, other.ptr_);
    detail::constexpr_swap(deleter_base::stored_value(), other.deleter_base::stored_value());
  }

private:
  pointer ptr_;
};

// Non-member comparison operators

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator==(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y) noexcept
{
  return x.get() == y.get();
}

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator!=(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y) noexcept
{
  return !(x == y);
}

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y)
{
  using CT = typename std::common_type<typename unique_ptr<T1, D1>::pointer, typename unique_ptr<T2, D2>::pointer>::type;
  return std::less<CT>{}(x.get(), y.get());
}

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y)
{
  return y < x;
}

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<=(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y)
{
  return !(y < x);
}

template<class T1, class D1, class T2, class D2>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>=(unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y)
{
  return !(x < y);
}

#if __cplusplus >= 202002L
template<class T1, class D1, class T2, class D2>
  requires std::three_way_comparable_with<typename unique_ptr<T1, D1>::pointer, typename unique_ptr<T2, D2>::pointer>
constexpr std::compare_three_way_result_t<typename unique_ptr<T1, D1>::pointer, typename unique_ptr<T2, D2>::pointer> operator<=>(
    unique_ptr<T1, D1> const& x, unique_ptr<T2, D2> const& y
)
{
  return std::compare_three_way{}(x.get(), y.get());
}
#endif

// Comparisons with nullptr

template<class T, class D>
constexpr bool operator==(unique_ptr<T, D> const& x, std::nullptr_t) noexcept
{
  return !x;
}

template<class T, class D>
constexpr bool operator==(std::nullptr_t, unique_ptr<T, D> const& x) noexcept
{
  return !x;
}

template<class T, class D>
constexpr bool operator!=(unique_ptr<T, D> const& x, std::nullptr_t) noexcept
{
  return static_cast<bool>(x);
}

template<class T, class D>
constexpr bool operator!=(std::nullptr_t, unique_ptr<T, D> const& x) noexcept
{
  return static_cast<bool>(x);
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<(unique_ptr<T, D> const& x, std::nullptr_t)
{
  return std::less<typename unique_ptr<T, D>::pointer>{}(x.get(), nullptr);
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<(std::nullptr_t, unique_ptr<T, D> const& x)
{
  return std::less<typename unique_ptr<T, D>::pointer>{}(nullptr, x.get());
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>(unique_ptr<T, D> const& x, std::nullptr_t)
{
  return nullptr < x;
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>(std::nullptr_t, unique_ptr<T, D> const& x)
{
  return x < nullptr;
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<=(unique_ptr<T, D> const& x, std::nullptr_t)
{
  return !(nullptr < x);
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator<=(std::nullptr_t, unique_ptr<T, D> const& x)
{
  return !(x < nullptr);
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>=(unique_ptr<T, D> const& x, std::nullptr_t)
{
  return !(x < nullptr);
}

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR bool operator>=(std::nullptr_t, unique_ptr<T, D> const& x)
{
  return !(nullptr < x);
}

#if __cplusplus >= 202002L
template<class T, class D>
  requires std::three_way_comparable<typename unique_ptr<T, D>::pointer>
constexpr std::compare_three_way_result_t<typename unique_ptr<T, D>::pointer> operator<=>(unique_ptr<T, D> const& x, std::nullptr_t)
{
  return std::compare_three_way{}(x.get(), static_cast<typename unique_ptr<T, D>::pointer>(nullptr));
}
#endif

// Non-member swap

template<class T, class D>
YK_POLYFILL_CXX14_CONSTEXPR void swap(unique_ptr<T, D>& x, unique_ptr<T, D>& y) noexcept(noexcept(x.swap(y)))
{
  x.swap(y);
}

// make_unique

template<class T, class... Args, class = typename std::enable_if<!std::is_array<T>::value>::type>
YK_POLYFILL_NODISCARD YK_POLYFILL_CXX20_CONSTEXPR unique_ptr<T> make_unique(Args&&... args)
{
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T, class = typename std::enable_if<is_unbounded_array<T>::value>::type>
YK_POLYFILL_NODISCARD YK_POLYFILL_CXX20_CONSTEXPR unique_ptr<T> make_unique(std::size_t size)
{
  return unique_ptr<T>(new typename std::remove_extent<T>::type[size]);
}

template<class T, class... Args, class = typename std::enable_if<is_bounded_array<T>::value>::type>
void make_unique(Args&&...) = delete;

namespace detail {

template<class T, class D, class = void>
struct unique_ptr_hash {};

template<class T, class D>
struct unique_ptr_hash<T, D, void_t<decltype(std::hash<typename unique_ptr<T, D>::pointer>{}(std::declval<typename unique_ptr<T, D>::pointer const&>()))>> {
  std::size_t operator()(unique_ptr<T, D> const& p) const
      noexcept(noexcept(std::hash<typename unique_ptr<T, D>::pointer>{}(p.get())))
  {
    return std::hash<typename unique_ptr<T, D>::pointer>{}(p.get());
  }
};

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

// Hash specialization
namespace std {

template<class T, class D>
struct hash<yk::polyfill::unique_ptr<T, D>> : yk::polyfill::detail::unique_ptr_hash<T, D> {};

}  // namespace std

#endif  // YK_ZZ_POLYFILL_MEMORY_HPP
