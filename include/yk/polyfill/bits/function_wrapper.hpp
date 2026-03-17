#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP

#include <yk/polyfill/bits/invoke.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace yk {

namespace polyfill {

namespace detail {

union bound_entity {
  void (*func_ptr)();
  void const* obj_ptr;

  struct func_tag {
    explicit func_tag() = default;
  };

  struct obj_tag {
    explicit obj_tag() = default;
  };

  template<class Func, typename std::enable_if<std::is_function<Func>::value, std::nullptr_t>::type = nullptr>
  bound_entity(func_tag, Func* f) : func_ptr(reinterpret_cast<void (*)()>(f))
  {
  }

  template<class Obj>
  YK_POLYFILL_CXX17_CONSTEXPR bound_entity(obj_tag, Obj&& obj) : obj_ptr(static_cast<void const*>(std::addressof(obj)))
  {
  }
};

template<bool Noexcept, class R, class... Args>
struct invoker {
  template<class Func>
  static R invoke_func(bound_entity entity, Args... args) noexcept(Noexcept)
  {
    return reinterpret_cast<Func*>(entity.func_ptr)(std::forward<Args>(args)...);
  }

  template<class Obj>
  static R invoke_obj(bound_entity entity, Args... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(*static_cast<Obj*>(const_cast<void*>(entity.obj_ptr)), std::forward<Args>(args)...);
  }
};

}  // namespace detail

template<class Signature>
class function_ref;

}  // namespace polyfill

}  // namespace yk

#define YK_POLYFILL_INCLUDE_FUNCTION_REF

// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

// #define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT
#include "yk/polyfill/bits/function_wrapper/function_ref.ipp"
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_CONST
#undef YK_POLYFILL_BITS_FUNCTION_WRAPPER_APPLY_NOEXCEPT

#undef YK_POLYFILL_INCLUDE_FUNCTION_REF

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
