#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_FUNCTION_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_FUNCTION_HPP

#include <yk/polyfill/bits/core_traits.hpp>
#include <yk/polyfill/bits/invoke.hpp>
#include <yk/polyfill/extension/specialization_of.hpp>
#include <yk/polyfill/utility.hpp>  // in_place_type_t

#include <functional>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

#include <cstddef>

namespace yk {

namespace polyfill {

template<class Signature>
class move_only_function;

namespace detail {

// Holds the target object in place if it is small enough, otherwise a pointer to it.
union mof_storage {
  void* obj;
  unsigned char buf[3 * sizeof(void*)];
};

// [func.wrap.move.general] notes that only nothrow-move-constructible targets can be stored in place.
template<class VT>
struct mof_is_local
    : bool_constant<sizeof(VT) <= sizeof(mof_storage) && alignof(mof_storage) % alignof(VT) == 0 && std::is_nothrow_move_constructible<VT>::value> {};

template<class T>
T* mof_launder(T* p) noexcept
{
#if __cpp_lib_launder >= 201606L
  return std::launder(p);
#else
  return p;
#endif
}

// Creates, accesses, and relocates or destroys a target object of type VT held in mof_storage.
// manage(src, dst) relocates the target into *dst if dst is non-null, otherwise destroys it.
template<class VT, bool = mof_is_local<VT>::value>
struct mof_holder {
  // [func.wrap.move.ctor] does not allow bad_alloc for these.
  static_assert(
      !(std::is_pointer<VT>::value && std::is_function<typename std::remove_pointer<VT>::type>::value)
          && !extension::is_specialization_of<VT, std::reference_wrapper>::value,
      "a function pointer or reference_wrapper must be stored in place"
  );

  template<class... Args>
  static void create(mof_storage& storage, Args&&... args)
  {
    storage.obj = ::new VT(std::forward<Args>(args)...);
  }

  static VT& get(mof_storage const& storage) noexcept { return *static_cast<VT*>(storage.obj); }

  static void manage(mof_storage& src, mof_storage* dst) noexcept
  {
    if (dst) {
      dst->obj = src.obj;
    } else {
      ::delete static_cast<VT*>(src.obj);
    }
  }
};

template<class VT>
struct mof_holder<VT, /* is_local = */ true> {
  template<class... Args>
  static void create(mof_storage& storage, Args&&... args)
  {
    ::new (static_cast<void*>(storage.buf)) VT(std::forward<Args>(args)...);
  }

  static VT& get(mof_storage const& storage) noexcept { return *mof_launder(reinterpret_cast<VT*>(const_cast<unsigned char*>(storage.buf))); }

  static void manage(mof_storage& src, mof_storage* dst) noexcept
  {
    VT& target = get(src);
    if (dst) ::new (static_cast<void*>(dst->buf)) VT(std::move(target));
    target.~VT();
  }
};

// [func.wrap.move.ctor] leaves *this with no target object for a null (member) pointer
// or an empty move_only_function.

template<class T, typename std::enable_if<std::is_pointer<T>::value || std::is_member_pointer<T>::value, std::nullptr_t>::type = nullptr>
constexpr bool mof_is_null(T const& f) noexcept
{
  return f == nullptr;
}

template<class T, typename std::enable_if<extension::is_specialization_of<T, move_only_function>::value, std::nullptr_t>::type = nullptr>
bool mof_is_null(T const& f) noexcept
{
  return !f;
}

template<
    class T, typename std::enable_if<
                 !(std::is_pointer<T>::value || std::is_member_pointer<T>::value || extension::is_specialization_of<T, move_only_function>::value),
                 std::nullptr_t>::type = nullptr>
constexpr bool mof_is_null(T const&) noexcept
{
  return false;
}

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#define YK_POLYFILL_INCLUDE_MOVE_ONLY_FUNCTION

#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#if __cpp_noexcept_function_type >= 201510L

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_LVALUE_REF
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_RVALUE_REF
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/move_only_function.ipp"

#endif

#undef YK_POLYFILL_INCLUDE_MOVE_ONLY_FUNCTION

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_FUNCTION_HPP
