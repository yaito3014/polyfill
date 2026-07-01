#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COMMON_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COMMON_HPP

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

#if __cplusplus >= 201703L
  // Stores a pointer value directly (as opposed to obj_tag, which stores addressof(obj)).
  struct ptr_tag {
    explicit ptr_tag() = default;
  };

  template<class T>
  constexpr bound_entity(ptr_tag, T* p) : obj_ptr(static_cast<void const*>(p))
  {
  }

  struct constant_tag {
    explicit constant_tag() = default;
  };

  constexpr bound_entity(constant_tag) : obj_ptr(nullptr) {}
#endif
};

template<bool Noexcept, class R, class... Args>
struct invoker {
  template<class Func>
  static R invoke_func(bound_entity entity, Args&&... args) noexcept(Noexcept)
  {
    return reinterpret_cast<Func*>(entity.func_ptr)(std::forward<Args>(args)...);
  }

  template<class Obj>
  static R invoke_obj(bound_entity entity, Args&&... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(*static_cast<Obj*>(const_cast<void*>(entity.obj_ptr)), std::forward<Args>(args)...);
  }

#if __cplusplus >= 201703L
  // Invokes the compile-time constant callable C, which is not stored in bound-entity.
  template<auto C>
  static R invoke_constant(bound_entity, Args&&... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(C, std::forward<Args>(args)...);
  }

  // Invokes C with a bound object whose address is stored in bound-entity (obj_tag).
  template<auto C, class Obj>
  static R invoke_constant_target(bound_entity entity, Args&&... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(C, *static_cast<Obj*>(const_cast<void*>(entity.obj_ptr)), std::forward<Args>(args)...);
  }

  // Invokes C with a bound pointer stored directly in bound-entity (ptr_tag).
  template<auto C, class Ptr>
  static R invoke_constant_pointer(bound_entity entity, Args&&... args) noexcept(Noexcept)
  {
    return polyfill::invoke_r<R>(C, static_cast<Ptr>(const_cast<void*>(entity.obj_ptr)), std::forward<Args>(args)...);
  }
#endif
};

}  // namespace detail

}  // namespace polyfill

}  // namespace yk

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_COMMON_HPP
