#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_FUNCTION_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_FUNCTION_HPP

#include <yk/polyfill/bits/invoke.hpp>
#include <yk/polyfill/extension/specialization_of.hpp>
#include <yk/polyfill/utility.hpp>  // in_place_type_t

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <cstddef>

namespace yk {

namespace polyfill {

template<class Signature>
class move_only_function;

namespace detail {

union mof_storage {
  void* obj;
  void (*func)();
};

// Holds a target object of type VT in mof_storage, by default allocated on the heap.
template<class VT, class = void>
struct mof_holder {
  template<class... Args>
  static void create(mof_storage& storage, Args&&... args)
  {
    storage.obj = ::new VT(std::forward<Args>(args)...);
  }

  static VT& get(mof_storage const& storage) noexcept { return *static_cast<VT*>(storage.obj); }

  static void destroy(mof_storage& storage) noexcept { ::delete static_cast<VT*>(storage.obj); }
};

// [func.wrap.move.ctor] allows bad_alloc unless VT is a function pointer or a specialization of reference_wrapper,
// so these two are stored in place as a pointer.

template<class Fn>
struct mof_holder<Fn*, typename std::enable_if<std::is_function<Fn>::value>::type> {
  static void create(mof_storage& storage, Fn* f) noexcept { storage.func = reinterpret_cast<void (*)()>(f); }

  static Fn* get(mof_storage const& storage) noexcept { return reinterpret_cast<Fn*>(storage.func); }

  static void destroy(mof_storage&) noexcept {}
};

template<class T, bool = std::is_function<T>::value>
struct mof_referent_storage {
  static void store(mof_storage& storage, T* p) noexcept { storage.obj = const_cast<void*>(static_cast<void const volatile*>(p)); }
  static T* load(mof_storage const& storage) noexcept { return static_cast<T*>(storage.obj); }
};

template<class T>
struct mof_referent_storage<T, /* is_function = */ true> {
  static void store(mof_storage& storage, T* p) noexcept { storage.func = reinterpret_cast<void (*)()>(p); }
  static T* load(mof_storage const& storage) noexcept { return reinterpret_cast<T*>(storage.func); }
};

template<class T>
struct mof_holder<std::reference_wrapper<T>> {
  static void create(mof_storage& storage, std::reference_wrapper<T> r) noexcept { mof_referent_storage<T>::store(storage, std::addressof(r.get())); }

  static std::reference_wrapper<T> get(mof_storage const& storage) noexcept { return std::reference_wrapper<T>(*mof_referent_storage<T>::load(storage)); }

  static void destroy(mof_storage&) noexcept {}
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
