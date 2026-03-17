#ifndef YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP

#include <yk/polyfill/config.hpp>

#include <yk/polyfill/functional.hpp>

#include <utility>

namespace yk {

namespace polyfill {

namespace detail {

template<bool Noex, class R, class... Args>
struct vtable {
  R (*invoke)(void*, Args...) YK_POLYFILL_CXX17_NOEXCEPT(Noex);
  void (*destroy)(void*);
};

template<class Obj, bool Noex, class R, class... Args>
struct vtable_dynamic {
  static constexpr vtable<Noex, R, Args...> value = {
      [](void* obj, Args... args) -> R { return polyfill::invoke(*static_cast<Obj*>(obj), std::forward<Args>(args)...); },
      [](void* obj) { delete static_cast<Obj*>(obj); },
  };
};

template<class Obj, bool Noex, class R, class... Args>
constexpr vtable<Noex, R, Args...> vtable_dynamic<Obj, Noex, R, Args...>::value;

}  // namespace detail

template<class Signature>
class move_only_function;

}  // namespace polyfill

}  // namespace yk

#define YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_IMPL_HPP
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_IMPL_CV
#define YK_POLYFILL_BITS_FUNCTION_WRAPPER_MOVE_ONLY_IMPL_REF

#include "yk/polyfill/bits/function_wrapper/move_only_impl.hpp"

#endif  // YK_ZZ_POLYFILL_BITS_FUNCTION_WRAPPER_HPP
