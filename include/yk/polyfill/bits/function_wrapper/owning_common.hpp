#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_OWNING_COMMON_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_OWNING_COMMON_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/extension/specialization_of.hpp>
#include <yk/polyfill/new.hpp>

#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include <cstddef>
#include <cstring>

namespace yk {

namespace polyfill {

template<class Signature>
class move_only_function;

namespace detail {

union owning_storage {
  void* obj;
  unsigned char buf[3 * sizeof(void*)];
};

// [func.wrap.move.general] and [func.wrap.copy.general] note that only nothrow-move-constructible targets can be stored
// in place.
template<class VT>
struct owning_is_local
    : bool_constant<sizeof(VT) <= sizeof(owning_storage) && alignof(owning_storage) % alignof(VT) == 0 && std::is_nothrow_move_constructible<VT>::value> {};

// ::new T(args...) and ::delete p, except for over-aligned T: before C++17 ::new cannot align it, and MSVC frees a
// throwing aligned new-expression with the non-aligned operator delete.
#if __cpp_aligned_new >= 201606L
#  if defined(_MSC_VER) && !defined(__clang__)
#    define YK_POLYFILL_DETAIL_OWNING_PLAIN_NEW_MAX_ALIGNMENT __STDCPP_DEFAULT_NEW_ALIGNMENT__
#  endif
#else
#  define YK_POLYFILL_DETAIL_OWNING_PLAIN_NEW_MAX_ALIGNMENT alignof(std::max_align_t)
#endif

template<class T>
struct owning_needs_explicit_new_delete
#ifdef YK_POLYFILL_DETAIL_OWNING_PLAIN_NEW_MAX_ALIGNMENT
    : bool_constant<(alignof(T) > YK_POLYFILL_DETAIL_OWNING_PLAIN_NEW_MAX_ALIGNMENT)>
#else
    : false_type
#endif
{
};

#undef YK_POLYFILL_DETAIL_OWNING_PLAIN_NEW_MAX_ALIGNMENT

template<class T, bool = owning_needs_explicit_new_delete<T>::value>
struct owning_global_new_delete {
  template<class... Args>
  static T* create(Args&&... args)
  {
    return ::new T(std::forward<Args>(args)...);
  }

  static void destroy(T* p) noexcept { ::delete p; }
};

template<class T>
struct owning_global_new_delete<T, /* needs_explicit_new_delete = */ true> {
  template<class... Args>
  static T* create(Args&&... args)
  {
    void* const p = allocate();
    try {
      return ::new (p) T(std::forward<Args>(args)...);
    } catch (...) {
      deallocate(p);
      throw;
    }
  }

  static void destroy(T* p) noexcept
  {
    p->~T();
    deallocate(p);
  }

private:
#if __cpp_aligned_new >= 201606L
  static void* allocate() { return ::operator new(sizeof(T), std::align_val_t{alignof(T)}); }

  static void deallocate(void* p) noexcept { ::operator delete(p, std::align_val_t{alignof(T)}); }
#else
  // The pointer that operator new returned is kept in front of the aligned storage, to be freed later.
  static void* allocate()
  {
    std::size_t space = sizeof(void*) + alignof(T) - 1 + sizeof(T);
    void* const raw = ::operator new(space);
    void* p = static_cast<unsigned char*>(raw) + sizeof(void*);
    space -= sizeof(void*);
    std::align(alignof(T), sizeof(T), p, space);
    std::memcpy(static_cast<unsigned char*>(p) - sizeof(void*), &raw, sizeof(void*));
    return p;
  }

  static void deallocate(void* p) noexcept
  {
    void* raw;
    std::memcpy(&raw, static_cast<unsigned char*>(p) - sizeof(void*), sizeof(void*));
    ::operator delete(raw);
  }
#endif
};

// relocate(src, dst) moves the target of src into dst, which has none, ending it in src.
template<class VT, bool = owning_is_local<VT>::value>
struct owning_holder {
  // [func.wrap.move.ctor] and [func.wrap.copy.ctor] do not allow bad_alloc for these.
  static_assert(
      !(std::is_pointer<VT>::value && std::is_function<typename std::remove_pointer<VT>::type>::value)
          && !extension::is_specialization_of<VT, std::reference_wrapper>::value,
      "a function pointer or reference_wrapper must be stored in place"
  );

  template<class... Args>
  static void create(owning_storage& storage, Args&&... args)
  {
    storage.obj = owning_global_new_delete<VT>::create(std::forward<Args>(args)...);
  }

  static VT& get(owning_storage const& storage) noexcept { return *static_cast<VT*>(storage.obj); }

  static void relocate(owning_storage& src, owning_storage& dst) noexcept { dst.obj = src.obj; }

  static void destroy(owning_storage& storage) noexcept { owning_global_new_delete<VT>::destroy(static_cast<VT*>(storage.obj)); }
};

template<class VT>
struct owning_holder<VT, /* is_local = */ true> {
  template<class... Args>
  static void create(owning_storage& storage, Args&&... args)
  {
    ::new (static_cast<void*>(storage.buf)) VT(std::forward<Args>(args)...);
  }

  static VT& get(owning_storage const& storage) noexcept { return *polyfill::launder(reinterpret_cast<VT*>(const_cast<unsigned char*>(storage.buf))); }

  static void relocate(owning_storage& src, owning_storage& dst) noexcept
  {
    VT& target = get(src);
    ::new (static_cast<void*>(dst.buf)) VT(std::move(target));
    target.~VT();
  }

  static void destroy(owning_storage& storage) noexcept { get(storage).~VT(); }
};

// [func.wrap.move.ctor] leaves *this with no target object for a null (member) pointer
// or an empty move_only_function.

template<class T, typename std::enable_if<std::is_pointer<T>::value || std::is_member_pointer<T>::value, std::nullptr_t>::type = nullptr>
constexpr bool owning_is_null(T const& f) noexcept
{
  return f == nullptr;
}

template<class T, typename std::enable_if<extension::is_specialization_of<T, move_only_function>::value, std::nullptr_t>::type = nullptr>
bool owning_is_null(T const& f) noexcept
{
  return !f;
}

template<
    class T, typename std::enable_if<
                 !(std::is_pointer<T>::value || std::is_member_pointer<T>::value || extension::is_specialization_of<T, move_only_function>::value),
                 std::nullptr_t>::type = nullptr>
constexpr bool owning_is_null(T const&) noexcept
{
  return false;
}

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_OWNING_COMMON_HPP
