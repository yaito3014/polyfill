#ifndef YK_ZZ_POLYFILL_NEW_HPP
#define YK_ZZ_POLYFILL_NEW_HPP

#include <yk/polyfill/config.hpp>

#include <new>
#include <type_traits>

// Before C++17, returning the pointer does not launder it; GCC and Clang provide the builtin in any mode, MSVC only
// from C++17.
#if __cpp_lib_launder >= 201606L
#elif defined(__has_builtin)
#if __has_builtin(__builtin_launder)
#define YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p) __builtin_launder(p)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 7
// GCC before 10 lacks __has_builtin.
#define YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p) __builtin_launder(p)
#endif

namespace yk {

namespace polyfill {

template<class T>
YK_POLYFILL_NODISCARD constexpr T* launder(T* p) noexcept
{
  static_assert(!std::is_function<T>::value && !std::is_void<T>::value, "[ptr.launder] mandates !is_function_v<T> && !is_void_v<T>");
#if __cpp_lib_launder >= 201606L
  return std::launder(p);
#elif defined(YK_POLYFILL_DETAIL_BUILTIN_LAUNDER)
  return YK_POLYFILL_DETAIL_BUILTIN_LAUNDER(p);
#else
  return p;
#endif
}

}  // namespace polyfill

}  // namespace yk

#undef YK_POLYFILL_DETAIL_BUILTIN_LAUNDER

#endif  // YK_ZZ_POLYFILL_NEW_HPP
