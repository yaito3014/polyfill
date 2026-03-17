#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP

#include <yk/polyfill/bits/invoke.hpp>

namespace yk {

namespace polyfill {

namespace detail {

union bound_entity {
  void const* obj_ptr;
  void (*func_ptr)();
};

template<bool Noexcept, class R, class... Args>
struct invoker {
  template<class Obj>
  static R invoke_obj(bound_entity entity, Args... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(*static_cast<Obj*>(const_cast<void*>(entity.obj_ptr)), std::forward<Args>(args)...);
  }

  template<class Func>
  static R invoke_func(bound_entity entity, Args... args) noexcept(Noexcept)
  {
    return reinterpret_cast<Func*>(entity.func_ptr)(std::forward<Args>(args)...);
  }
};

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
